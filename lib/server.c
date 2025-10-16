#include "server.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFSIZE 1024

/*
    Server
*/

Server *sr_create_server(unsigned short port) {
    Server *server = malloc(sizeof(Server));

    if (!server) {
        perror("failed to allocate memory for server");
        return NULL;
    }

    server->addrlen = sizeof(server->servaddr);
    bzero(&server->servaddr, sizeof(server->servaddr));

    if ((server->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        free(server);
        return NULL;
    }

    server->servaddr.sin_family = AF_INET;
    server->servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    server->servaddr.sin_port = htons(port);

    int optval = 1;
    setsockopt(
        server->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)
    );

    if (bind(
            server->fd,
            (const struct sockaddr *)&server->servaddr,
            sizeof(server->servaddr)
        )
        < 0) {
        perror("bind failed");
        close(server->fd);
        free(server);
        return NULL;
    }

    server->client_count = 0;
    pthread_mutex_init(&server->clients_mutex, NULL);

    for (size_t i = 0; i < MAX_PLAYERS; i++) {
        server->clients[i].active = 0;
    }

    return server;
}

void sr_destroy_server(Server *server) {
    if (!server) {
        return;
    }

    pthread_mutex_lock(&server->clients_mutex);
    for (size_t i = 0; i < server->client_count; i++) {
        if (server->clients[i].active) {
            server->clients[i].active = 0;
            close(server->clients[i].socket_fd);
            pthread_join(server->clients[i].thread_id, NULL);
        }
    }
    pthread_mutex_unlock(&server->clients_mutex);
    pthread_mutex_destroy(&server->clients_mutex);
    close(server->fd);
    free(server);
}

void sr_start_listen(Server *server) {
    if (listen(server->fd, MAX_PLAYERS) < 0) {
        perror("failed listening");
        return;
    }

    printf(
        "Server listening on port %d...\n", ntohs(server->servaddr.sin_port)
    );
    printf("Waiting for connections...\n");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t          client_len = sizeof(client_addr);

        int client_fd =
            accept(server->fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_fd < 0) {
            perror("failed to accept connection");
            continue;
        }

        printf(
            "New connection accepted from %s\n", inet_ntoa(client_addr.sin_addr)
        );

        int client_id = sr_add_client(server, client_fd, client_addr);

        if (client_id < 0) {
            printf("Failed to add client (server full or error)\n");
            close(client_fd);
            continue;
        }

        printf("Successfully added client with ID %d\n", client_id);
    }
}

void *handle_client(void *arg) {
    ThreadArgs       *args = (ThreadArgs *)arg;
    Server           *server = args->server;
    ClientConnection *con = &server->clients[args->client_index];

    printf(
        "Client %d connected from %s\n",
        con->client_id,
        inet_ntoa(con->addr.sin_addr)
    );

    if (con->client_id == 0) {
        ServerMessage is_main_msg = {
            .type = SERVER_MSG_IS_MAIN,
            .client_id = con->client_id,
            .timestamp = time(NULL),
        };

        sr_send_message_to_client(server, con->client_id, &is_main_msg);
    }

    while (con->active) {
        ClientMessage msg;
        ssize_t bytes_read = read(con->socket_fd, &msg, sizeof(ClientMessage));

        if (bytes_read <= 0) {
            printf("Client %d disconnected\n", con->client_id);
            break;
        }

        if (bytes_read == sizeof(ClientMessage)) {
            switch (msg.type) {
                case CLIENT_MSG_PADDLE_POSITION: {
                    printf(
                        "Client %d position: (%.2f, %.2f)\n",
                        con->client_id,
                        msg.data.position.x,
                        msg.data.position.y
                    );

                    con->paddle_position = msg.data.position;

                    ServerMessage pos_broadcast = {
                        .type = SERVER_MSG_PADDLE_POSITION_UPDATE,
                        .client_id = con->client_id,
                        .timestamp = time(NULL),
                        .data.position = msg.data.position
                    };
                    sr_send_message_to_all_except(
                        server, con->client_id, &pos_broadcast
                    );
                    break;
                }
                case CLIENT_MSG_BALL_POSITION: {
                    if (con->client_id != 0) {
                        break;
                    }

                    printf(
                        "Ball from client: %d position: (%.2f, %.2f)\n",
                        con->client_id,
                        msg.data.position.x,
                        msg.data.position.y
                    );

                    con->ball_position = msg.data.position;

                    ServerMessage pos_broadcast = {
                        .type = SERVER_MSG_BALL_POSITION_UPDATE,
                        .client_id = con->client_id,
                        .timestamp = time(NULL),
                        .data.position = msg.data.position
                    };
                    sr_send_message_to_all_except(
                        server, con->client_id, &pos_broadcast
                    );
                    break;
                }
                default: {
                    printf(
                        "Unknown message type %d from client %d\n",
                        msg.type,
                        con->client_id
                    );
                    break;
                }
            }
        }
    }

    ServerMessage goodbye_msg = { .type = SERVER_MSG_PLAYER_LEFT,
                                  .client_id = con->client_id,
                                  .timestamp = time(NULL) };
    sr_send_message_to_all_except(server, con->client_id, &goodbye_msg);

    close(con->socket_fd);
    con->active = 0;

    pthread_mutex_lock(&server->clients_mutex);
    server->client_count--;
    pthread_mutex_unlock(&server->clients_mutex);

    free(arg);
    return NULL;
}

int sr_add_client(Server *server, int socket_fd, struct sockaddr_in addr) {
    if (server->client_count >= MAX_PLAYERS) {
        return 1;
    }

    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        ClientConnection *con = &server->clients[i];

        if (con->active) {
            continue;
        }

        con->socket_fd = socket_fd;
        con->addr = addr;
        con->active = 1;
        con->client_id = i;

        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->client_index = i;
        args->server = server;

        if (pthread_create(
                &server->clients[i].thread_id, NULL, handle_client, (void *)args
            )
            < 0) {
            con->active = 0;
            free(args);
            pthread_mutex_unlock(&server->clients_mutex);
            return -1;
        }

        server->client_count++;
        pthread_mutex_unlock(&server->clients_mutex);
        return con->client_id;
    }

    pthread_mutex_unlock(&server->clients_mutex);
    return -1;
}

void sr_send_message_to_all(Server *server, const ServerMessage *message) {
    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (server->clients[i].active) {
            send(
                server->clients[i].socket_fd, message, sizeof(ServerMessage), 0
            );
        }
    }

    pthread_mutex_unlock(&server->clients_mutex);
}

void sr_send_message_to_all_except(
    Server              *server,
    int                  except_client_id,
    const ServerMessage *message
) {
    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (server->clients[i].active
            && server->clients[i].client_id != except_client_id) {
            send(
                server->clients[i].socket_fd, message, sizeof(ServerMessage), 0
            );
        }
    }

    pthread_mutex_unlock(&server->clients_mutex);
}

void sr_send_message_to_client(
    Server              *server,
    int                  client_id,
    const ServerMessage *message
) {
    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (server->clients[i].active
            && server->clients[i].client_id == client_id) {
            send(
                server->clients[i].socket_fd, message, sizeof(ServerMessage), 0
            );
            break;
        }
    }

    pthread_mutex_unlock(&server->clients_mutex);
}

/*
    Client
*/

int sr_client_connect(
    Client        *client,
    const char    *server_ip,
    unsigned short port
) {
    if (!server_ip) {
        printf("ip is NULL\n");
        return -1;
    }

    client->addr_len = sizeof(client->server_addr);

    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);
    client->server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (inet_pton(AF_INET, server_ip, &client->server_addr.sin_addr) <= 0) {
        printf("invalid address or address not supported\n");
        return -1;
    }

    if ((client->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket creation failed...\n");
        return -1;
    }

    if (connect(
            client->server_fd,
            (struct sockaddr *)&client->server_addr,
            sizeof(client->server_addr)
        )
        < 0) {
        printf("connection with the server failed...\n");
        return -1;
    }

    printf("connected to the server..\n");
    return 0;
}

void sr_client_close(Client *client) {
    close(client->server_fd);
}

int sr_receive_server_message(Client *client, ServerMessage *msg) {
    if (!client || !msg) {
        return -1;
    }

    static int made_nonblocking = 0;

    if (!made_nonblocking) {
        int flags = fcntl(client->server_fd, F_GETFL, 0);
        fcntl(client->server_fd, F_SETFL, flags | O_NONBLOCK);
        made_nonblocking = 1;
    }

    ssize_t bytes_read = recv(client->server_fd, msg, sizeof(ServerMessage), 0);

    if (bytes_read == sizeof(ServerMessage)) {
        return 0;
    } else if (bytes_read == 0) {
        return -1;
    } else if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -3;
        }
        return -2;
    } else {
        return -2;
    }
}

int sr_send_message_to_server(Client *client, const ClientMessage *msg) {
    if (!client || !msg) {
        return -1;
    }

    ssize_t bytes_sent = send(client->server_fd, msg, sizeof(ClientMessage), 0);

    if (bytes_sent != sizeof(ClientMessage)) {
        perror("ERROR in sr_send_message_to_server");
        return -2;
    }

    return 0;
}
