#ifndef SERVER_H
#define SERVER_H

#include "utils.h"
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>

#define MAGIC_NUMBER 0x52351aE
#define MAX_PLAYERS  2

typedef enum {
    CLIENT_MSG_PADDLE_POSITION = 1,
    CLIENT_MSG_BALL_POSITION = 2,
    CLIENT_MSG_DISCONNECT = 3,
    CLIENT_MSG_REGISTER = 4,
    CLIENT_MSG_HEARTBEAT = 5,
} ClientMessageType;

typedef struct {
    ClientMessageType type;
    int               client_id;
    uint32_t          timestamp;

    union {
        vec2 position;
        char text[256];
    } data;
} ClientMessage;

typedef enum {
    SERVER_MSG_PADDLE_POSITION_UPDATE = 1,
    SERVER_MSG_BALL_POSITION_UPDATE = 2,
    SERVER_MSG_PLAYER_JOINED = 3,
    SERVER_MSG_PLAYER_LEFT = 4,
    SERVER_MSG_IS_MAIN = 5,
    SERVER_MSG_REGISTER_CONFIRM = 6,
} ServerMessageType;

typedef struct {
    ServerMessageType type;
    int               client_id;
    uint32_t          timestamp;

    union {
        vec2 position;
        char text[256];
        struct {
            int  player_id;
            char player_name[32];
        } player_info;
    } data;
} ServerMessage;

typedef struct {
    int                server_fd;
    struct sockaddr_in server_addr;
    socklen_t          addr_len;
} Client;

typedef struct {
    int                socket_fd;
    struct sockaddr_in addr;
    int                active;
    int                is_main;
    pthread_t          thread_id;
    int                client_id;

    vec2 paddle_position;
    vec2 ball_position;
} ClientConnection;

typedef struct {
    int                fd;
    struct sockaddr_in servaddr;
    socklen_t          addrlen;

    ClientConnection clients[MAX_PLAYERS];
    size_t           client_count;
    pthread_mutex_t  clients_mutex;
} Server;

typedef struct {
    Server *server;
    int     client_index;
} ThreadArgs;

typedef struct {
    int                server_fd;
    struct sockaddr_in server_addr;
    socklen_t          addr_len;
    int                client_id;
    int                registered;
    int                is_main;
} UDPClient;

typedef struct {
    struct sockaddr_in addr;
    socklen_t          addr_len;
    int                active;
    int                client_id;
    uint32_t           last_seen;
} UDPClientConnection;

typedef struct {
    int                fd;
    struct sockaddr_in servaddr;
    socklen_t          addrlen;

    UDPClientConnection clients[MAX_PLAYERS];
    int                 client_count;
    pthread_mutex_t     clients_mutex;
} UDPServer;

Server *sr_create_server(unsigned short port);
void    sr_destroy_server(Server *server);
void    sr_start_listen(Server *server);
int     sr_add_client(Server *server, int socket_fd, struct sockaddr_in addr);
int     sr_client_connect(
        Client        *client,
        const char    *server_ip,
        unsigned short port
    );
void sr_send_message_to_all(Server *server, const ServerMessage *message);
void sr_send_message_to_all_except(
    Server              *server,
    int                  except_client_id,
    const ServerMessage *message
);
void sr_send_message_to_client(
    Server              *server,
    int                  client_id,
    const ServerMessage *message
);

int  sr_send_message_to_server(Client *client, const ClientMessage *msg);
void sr_client_close(Client *client);
int  sr_receive_server_message(Client *client, ServerMessage *msg);

UDPServer *sr_create_udp_server(unsigned short port);
void       sr_destroy_udp_server(UDPServer *server);
void       sr_start_udp_listen(UDPServer *server);
int        sr_add_udp_client(
           UDPServer         *server,
           struct sockaddr_in addr,
           socklen_t          addr_len
       );
void sr_remove_udp_client(UDPServer *server, int client_id);
void sr_check_client_timeouts(UDPServer *server);
int  sr_find_client_by_addr(UDPServer *server, struct sockaddr_in *addr);
void sr_handle_udp_message(
    UDPServer     *server,
    int            client_id,
    ClientMessage *msg
);

void sr_send_udp_message_to_all(
    UDPServer           *server,
    const ServerMessage *message
);
void sr_send_udp_message_to_all_except(
    UDPServer           *server,
    int                  except_client_id,
    const ServerMessage *message
);
void sr_send_udp_message_to_client(
    UDPServer           *server,
    int                  client_id,
    const ServerMessage *message
);

int sr_udp_client_connect(
    UDPClient     *client,
    const char    *server_ip,
    unsigned short port
);
void sr_udp_client_close(UDPClient *client);
int  sr_send_udp_message_to_server(UDPClient *client, const ClientMessage *msg);
int  sr_receive_udp_server_message(UDPClient *client, ServerMessage *msg);

#endif  // SERVER_H
