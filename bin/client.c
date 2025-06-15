#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define MAXLINE 1024

typedef struct vec2 {
  float x;
  float y;
} vec2;

typedef struct position_packet_t {
  vec2 position;
} position_packet_t;

int main() {
  int sockfd;
  char buffer[MAXLINE];
  struct sockaddr_in servaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    return 1;
  }

  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = INADDR_ANY;

  int n;
  socklen_t len;

  char write_buffer[MAXLINE];

  position_packet_t packet = {.position = {.x = 0.5f, .y = -3.f}};

  memcpy(write_buffer, &packet, sizeof(packet));

  ssize_t sent = sendto(sockfd, &packet, sizeof(packet), 0,
                        (const struct sockaddr *)&servaddr, sizeof(servaddr));

  if (sent < 0) {
    perror("sendto failed");
    close(sockfd);
    return 1;
  }

  printf("Message sent with %zd bytes\n", sent);

  close(sockfd);
  return 0;
}
