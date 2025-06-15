#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "server.h"

#define PORT 8080
#define MAXLINE 1024

int main(void) {
  Server *server = sr_create_server(PORT);

  if (!server) {
    fprintf(stderr, "error: Failed to create server\n");
    return 1;
  }

  sr_start_listen(server);

  return 0;
}
