#ifndef SERVER_H
#define SERVER_H

#include "utils.h"
#include <netinet/in.h>
#include <unistd.h>

#define MAGIC_NUMBER 0x52351aE

typedef enum {
  SERVER_SUCCESS = 0,
} ServerResult;

typedef struct {
  int magic;
  int player_id;
  vec2 position;
} PositionPacket;

typedef struct {
  int fd;
} Server;

typedef struct {
  int fd;
  struct sockaddr_in addr;
  socklen_t addr_len;
} Client;

Server *sr_create_server(unsigned short port);
void sr_destroy_server(Server *server);

void sr_start_listen(Server *server);

Client *sr_client_create(const char *ip, unsigned short port);
void sr_client_destroy(Client *client);

void sr_send_client_position(Client *client, vec2 position);

#endif // !SERVER_H
