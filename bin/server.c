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

int main(void) {
  int sockfd;
  position_packet_t received_packet;
  struct sockaddr_in servaddr, cliaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    return 1;
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    return 1;
  }

  printf("Server started\n");

  socklen_t len = sizeof(cliaddr);

  ssize_t n = recvfrom(sockfd, &received_packet, sizeof(received_packet), 0,
                       (struct sockaddr *)&cliaddr, &len);

  if (n == sizeof(received_packet)) {
    printf("Received packet: x: %.2f, y: %.2f\n", received_packet.position.x,
           received_packet.position.y);
  } else {
    printf("Received packet with wrong size, expected: %zu actual: %zu\n",
           sizeof(received_packet), n);
  }

  return 0;
}
