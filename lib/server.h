#ifndef SERVER_H
#define SERVER_H

#include "utils.h"
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <unistd.h>

#define MAGIC_NUMBER 0x52351aE
#define MAX_PLAYERS 2

typedef enum {
  CLIENT_MSG_POSITION = 1,
  CLIENT_MSG_DISCONNECT = 2
} ClientMessageType;

typedef struct {
  ClientMessageType type;
  int client_id;

  union {
    vec2 position;
    char text[256];
  } data;
} ClientMessage;

typedef enum {
  SERVER_MSG_POSITION_UPDATE = 1,
  SERVER_MSG_PLAYER_JOINED = 2,
  SERVER_MSG_PLAYER_LEFT = 3,
  SERVER_MSG_GAME_STATE = 4
} ServerMessageType;

typedef struct {
  ServerMessageType type;
  int client_id;
  uint32_t timestamp;
  union {
    vec2 position;
    char text[256];
    struct {
      int player_id;
      char player_name[32];
    } player_info;
  } data;
} ServerMessage;

typedef struct {
  int server_fd;
  struct sockaddr_in server_addr;
  socklen_t addr_len;
} Client;

typedef struct {
  int socket_fd;
  struct sockaddr_in addr;
  int active;
  int is_main;
  pthread_t thread_id;
  int client_id;

  vec2 position;
} ClientConnection;

typedef struct {
  int fd;
  struct sockaddr_in servaddr;
  socklen_t addrlen;

  ClientConnection clients[MAX_PLAYERS];
  size_t client_count;
  pthread_mutex_t clients_mutex;
} Server;

typedef struct {
  Server *server;
  int client_index;
} ThreadArgs;

Server *sr_create_server(unsigned short port);
void sr_destroy_server(Server *server);
void sr_start_listen(Server *server);
int sr_add_client(Server *server, int socket_fd, struct sockaddr_in addr);
int sr_client_connect(Client *client, const char *server_ip,
                      unsigned short port);
void sr_send_message_to_all(Server *server, const ServerMessage *message);
void sr_send_message_to_all_except(Server *server, int except_client_id,
                                   const ServerMessage *message);
void sr_send_message_to_client(Server *server, int client_id,
                               const ServerMessage *message);

void sr_send_position_to_server(Client *client, vec2 position);
void sr_client_close(Client *client);

int sr_receive_server_message(Client *client, ServerMessage *msg);

#endif // !SERVER_H
