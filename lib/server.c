#include "server.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 1024

Server *sr_create_server(unsigned short port) {
  Server *server = malloc(sizeof(Server));
  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));

  if (!server) {
    perror("failed to allocate memory for server");
    return NULL;
  }

  if ((server->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    return NULL;
  }

  int optval = 1;
  setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
             sizeof(int));

  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  servaddr.sin_family = AF_INET;

  if (bind(server->fd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) <
      0) {
    perror("bind failed");
    return NULL;
  }

  return server;
}

void sr_destroy_server(Server *server) {
  if (!server) {
    return;
  }

  free(server);
}

void sr_start_listen(Server *server) {
  struct sockaddr_in clientaddr;
  socklen_t clientlen = sizeof(clientaddr);
  char buf[BUFSIZE];

  while (1) {
    bzero(buf, BUFSIZE);
    ssize_t bytes_read = recvfrom(server->fd, buf, BUFSIZE, 0,
                                  (struct sockaddr *)&clientaddr, &clientlen);
    if (bytes_read < 0) {
      perror("ERROR in recvfrom");
      return;
    }

    struct hostent *hostp =
        gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                      sizeof(clientaddr.sin_addr.s_addr), AF_INET);

    if (hostp == NULL) {
      perror("ERROR on gethostbyaddr");
      return;
    }

    char *hostaddrp = inet_ntoa(clientaddr.sin_addr);

    if (hostaddrp == NULL) {
      perror("ERROR on inet_ntoa\n");
      return;
    }

    printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);

    if (bytes_read == sizeof(vec2)) {
      vec2 *position = (vec2 *)buf;
      printf("server received position: x=%.2f, y=%.2f\n", position->x,
             position->y);
    } else {
      printf("server received %lu/%zd bytes: %s\n", strlen(buf), bytes_read,
             buf);
    }

    bytes_read = sendto(server->fd, buf, bytes_read, 0,
                        (struct sockaddr *)&clientaddr, clientlen);

    if (bytes_read < 0) {
      perror("ERROR in sendto");
      return;
    }
  }

  close(server->fd);
}

Client *sr_client_create(const char *ip, unsigned short port) {
  if (!ip) {
    return NULL;
  }

  Client *client = malloc(sizeof(Client));

  if (!client) {
    perror("failed to allocate memory for client");
    return NULL;
  }

  client->fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (client->fd < 0) {
    perror("socket creation failed for client");
    free(client);
    return NULL;
  }

  client->addr_len = sizeof(struct sockaddr_in);

  bzero(&client->addr, sizeof(client->addr));
  client->addr.sin_family = AF_INET;
  client->addr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip, &client->addr.sin_addr) <= 0) {
    perror("invalid IP address");
    close(client->fd);
    free(client);
    return NULL;
  }

  printf("Created client for %s:%d with fd %d\n", ip, port, client->fd);

  return client;
}

void sr_client_destroy(Client *client) {
  if (!client) {
    return;
  }

  printf("Destroying client %s:%d\n", inet_ntoa(client->addr.sin_addr),
         ntohs(client->addr.sin_port));

  close(client->fd);
  free(client);
}

void sr_send_client_position(Client *client, vec2 position) {
  if (!client) {
    return;
  }

  ssize_t bytes_sent =
      sendto(client->fd, &position, sizeof(vec2), 0,
             (struct sockaddr *)&client->addr, client->addr_len);

  if (bytes_sent < 0) {
    perror("ERROR in rv_send_client_position");
    return;
  }

  printf("Sent position (%.2f, %.2f) to client\n", position.x, position.y);
}
