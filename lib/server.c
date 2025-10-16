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

#define BUFSIZE                  1024
#define UDP_REGISTRATION_TIMEOUT 1
#define DISCONNECT_TIMEOUT       30
/*
    TCP Server
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
    TCP Client
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

/*
    UDP Server
*/

UDPServer *sr_create_udp_server(unsigned short port) {
    UDPServer *server = malloc(sizeof(UDPServer));

    if (!server) {
        perror("failed to allocate memory for server");
        return NULL;
    }

    server->addrlen = sizeof(server->servaddr);
    bzero(&server->servaddr, sizeof(server->servaddr));

    if ((server->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
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
        server->clients[i].last_seen = time(NULL);
    }

    printf("UDP Server created on port %d\n", port);
    return server;
}

void sr_destroy_udp_server(UDPServer *server) {
    if (!server) {
        return;
    }

    pthread_mutex_destroy(&server->clients_mutex);
    close(server->fd);
    free(server);
}

void sr_start_udp_listen(UDPServer *server) {
    printf(
        "UDP Server listening on port %d...\n", ntohs(server->servaddr.sin_port)
    );
    printf("Waiting for datagrams...\n");

    char buffer[sizeof(ClientMessage)];

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t          client_len = sizeof(client_addr);

        ssize_t recv_len = recvfrom(
            server->fd,
            buffer,
            sizeof(ClientMessage),
            0,
            (struct sockaddr *)&client_addr,
            &client_len
        );

        if (recv_len < 0) {
            perror("recvfrom failed");
            continue;
        }

        if (recv_len == sizeof(ClientMessage)) {
            ClientMessage *msg = (ClientMessage *)buffer;

            if (msg->type == CLIENT_MSG_REGISTER) {
                int client_id =
                    sr_add_udp_client(server, client_addr, client_len);

                if (client_id >= 0) {
                    printf(
                        "New client registered with ID %d from %s:%d\n",
                        client_id,
                        inet_ntoa(client_addr.sin_addr),
                        ntohs(client_addr.sin_port)
                    );

                    ServerMessage confirm_msg = {
                        .type = SERVER_MSG_REGISTER_CONFIRM,
                        .client_id = client_id,
                        .timestamp = time(NULL)
                    };

                    sr_send_udp_message_to_client(
                        server, client_id, &confirm_msg
                    );

                    if (client_id == 0) {
                        ServerMessage is_main_msg = {
                            .type = SERVER_MSG_IS_MAIN,
                            .client_id = client_id,
                            .timestamp = time(NULL),
                        };
                        printf(
                            "Sending IS_MAIN message to client %d (first player)\n",
                            client_id
                        );
                        sr_send_udp_message_to_client(
                            server, client_id, &is_main_msg
                        );
                    }

                    ServerMessage new_player_msg = {
                        .type = SERVER_MSG_PLAYER_JOINED,
                        .client_id = client_id,
                        .timestamp = time(NULL)
                    };
                    sr_send_udp_message_to_all_except(
                        server, client_id, &new_player_msg
                    );
                } else {
                    printf("Failed to register new client - server full\n");

                    ServerMessage reject_msg = {
                        .type = SERVER_MSG_REGISTER_CONFIRM,
                        .client_id = -1,
                        .timestamp = time(NULL)
                    };

                    sendto(
                        server->fd,
                        &reject_msg,
                        sizeof(ServerMessage),
                        0,
                        (struct sockaddr *)&client_addr,
                        client_len
                    );
                }
            } else {
                int client_id = sr_find_client_by_addr(server, &client_addr);

                if (client_id >= 0) {
                    pthread_mutex_lock(&server->clients_mutex);
                    for (int i = 0; i < MAX_PLAYERS; i++) {
                        if (server->clients[i].client_id == client_id) {
                            server->clients[i].last_seen = time(NULL);
                            break;
                        }
                    }
                    pthread_mutex_unlock(&server->clients_mutex);

                    sr_handle_udp_message(server, client_id, msg);
                } else {
                    printf("Received message from unregistered client\n");

                    ServerMessage reject_msg = {
                        .type = SERVER_MSG_REGISTER_CONFIRM,
                        .client_id = -1,
                        .timestamp = time(NULL)
                    };

                    sendto(
                        server->fd,
                        &reject_msg,
                        sizeof(ServerMessage),
                        0,
                        (struct sockaddr *)&client_addr,
                        client_len
                    );
                }
            }
        } else {
            printf("Received incomplete message of size %zd\n", recv_len);
        }

        sr_check_client_timeouts(server);
    }
}

void sr_handle_udp_message(
    UDPServer     *server,
    int            client_id,
    ClientMessage *msg
) {
    switch (msg->type) {
        case CLIENT_MSG_PADDLE_POSITION: {
            printf(
                "Client %d position: (%.2f, %.2f)\n",
                client_id,
                msg->data.position.x,
                msg->data.position.y
            );

            ServerMessage pos_broadcast = {
                .type = SERVER_MSG_PADDLE_POSITION_UPDATE,
                .client_id = client_id,
                .timestamp = time(NULL),
                .data.position = msg->data.position
            };
            sr_send_udp_message_to_all_except(
                server, client_id, &pos_broadcast
            );
            break;
        }
        case CLIENT_MSG_BALL_POSITION: {
            if (client_id != 0) {
                printf(
                    "Warning: Non-main client %d tried to send ball position\n",
                    client_id
                );
                break;
            }

            printf(
                "Ball from client %d: (%.2f, %.2f)\n",
                client_id,
                msg->data.position.x,
                msg->data.position.y
            );

            ServerMessage pos_broadcast = { .type =
                                                SERVER_MSG_BALL_POSITION_UPDATE,
                                            .client_id = client_id,
                                            .timestamp = time(NULL),
                                            .data.position =
                                                msg->data.position };
            sr_send_udp_message_to_all_except(
                server, client_id, &pos_broadcast
            );
            break;
        }
        case CLIENT_MSG_HEARTBEAT: {
            printf("Heartbeat from client %d\n", client_id);
            break;
        }
        case CLIENT_MSG_DISCONNECT: {
            printf("Client %d disconnected\n", client_id);
            sr_remove_udp_client(server, client_id);

            ServerMessage disconnect_msg = { .type = SERVER_MSG_PLAYER_LEFT,
                                             .client_id = client_id,
                                             .timestamp = time(NULL) };
            sr_send_udp_message_to_all(server, &disconnect_msg);
            break;
        }
        default: {
            printf(
                "Unknown message type %d from client %d\n", msg->type, client_id
            );
            break;
        }
    }
}

int sr_add_udp_client(
    UDPServer         *server,
    struct sockaddr_in addr,
    socklen_t          addr_len
) {
    if (server->client_count >= MAX_PLAYERS) {
        printf("Server full, cannot add new client\n");
        return -1;
    }

    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        UDPClientConnection *con = &server->clients[i];

        if (con->active && con->addr.sin_addr.s_addr == addr.sin_addr.s_addr
            && con->addr.sin_port == addr.sin_port) {
            pthread_mutex_unlock(&server->clients_mutex);
            return con->client_id;
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        UDPClientConnection *con = &server->clients[i];

        if (con->active) {
            continue;
        }

        con->addr = addr;
        con->addr_len = addr_len;
        con->active = 1;
        con->client_id = i;
        con->last_seen = time(NULL);

        server->client_count++;
        pthread_mutex_unlock(&server->clients_mutex);
        return con->client_id;
    }

    pthread_mutex_unlock(&server->clients_mutex);
    return -1;
}

void sr_remove_udp_client(UDPServer *server, int client_id) {
    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        UDPClientConnection *con = &server->clients[i];
        if (con->active && con->client_id == client_id) {
            con->active = 0;
            server->client_count--;
            break;
        }
    }

    pthread_mutex_unlock(&server->clients_mutex);
}

void sr_check_client_timeouts(UDPServer *server) {
    time_t current_time = time(NULL);
    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        UDPClientConnection *con = &server->clients[i];
        if (con->active
            && (current_time - con->last_seen) > DISCONNECT_TIMEOUT) {
            printf("Client %d timed out\n", con->client_id);
            con->active = 0;
            server->client_count--;

            ServerMessage timeout_msg = { .type = SERVER_MSG_PLAYER_LEFT,
                                          .client_id = con->client_id,
                                          .timestamp = current_time };

            pthread_mutex_unlock(&server->clients_mutex);
            sr_send_udp_message_to_all(server, &timeout_msg);
            pthread_mutex_lock(&server->clients_mutex);
        }
    }

    pthread_mutex_unlock(&server->clients_mutex);
}

int sr_find_client_by_addr(UDPServer *server, struct sockaddr_in *addr) {
    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        UDPClientConnection *con = &server->clients[i];

        if (con->active && con->addr.sin_addr.s_addr == addr->sin_addr.s_addr
            && con->addr.sin_port == addr->sin_port) {
            pthread_mutex_unlock(&server->clients_mutex);
            return con->client_id;
        }
    }

    pthread_mutex_unlock(&server->clients_mutex);
    return -1;
}

void sr_send_udp_message_to_all(
    UDPServer           *server,
    const ServerMessage *message
) {
    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (server->clients[i].active) {
            sendto(
                server->fd,
                message,
                sizeof(ServerMessage),
                0,
                (struct sockaddr *)&server->clients[i].addr,
                server->clients[i].addr_len
            );
        }
    }

    pthread_mutex_unlock(&server->clients_mutex);
}

void sr_send_udp_message_to_all_except(
    UDPServer           *server,
    int                  except_client_id,
    const ServerMessage *message
) {
    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (server->clients[i].active
            && server->clients[i].client_id != except_client_id) {
            sendto(
                server->fd,
                message,
                sizeof(ServerMessage),
                0,
                (struct sockaddr *)&server->clients[i].addr,
                server->clients[i].addr_len
            );
        }
    }

    pthread_mutex_unlock(&server->clients_mutex);
}

void sr_send_udp_message_to_client(
    UDPServer           *server,
    int                  client_id,
    const ServerMessage *message
) {
    pthread_mutex_lock(&server->clients_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (server->clients[i].active
            && server->clients[i].client_id == client_id) {
            sendto(
                server->fd,
                message,
                sizeof(ServerMessage),
                0,
                (struct sockaddr *)&server->clients[i].addr,
                server->clients[i].addr_len
            );
            break;
        }
    }

    pthread_mutex_unlock(&server->clients_mutex);
}

/*
    UDP Client
*/

int sr_udp_client_connect(
    UDPClient     *client,
    const char    *server_ip,
    unsigned short port
) {
    if (!server_ip) {
        printf("ip is NULL\n");
        return -1;
    }

    client->addr_len = sizeof(client->server_addr);
    bzero(&client->server_addr, client->addr_len);

    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &client->server_addr.sin_addr) <= 0) {
        printf("invalid address or address not supported\n");
        return -1;
    }

    if ((client->server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket creation failed...\n");
        return -1;
    }

    int flags = fcntl(client->server_fd, F_GETFL, 0);

    fcntl(client->server_fd, F_SETFL, flags | O_NONBLOCK);

    client->registered = 0;
    client->client_id = -1;

    printf(
        "UDP client created, ready to send to server %s:%d\n", server_ip, port
    );

    ClientMessage register_msg = { .type = CLIENT_MSG_REGISTER,
                                   .timestamp = time(NULL) };

    if (sr_send_udp_message_to_server(client, &register_msg) < 0) {
        printf("Failed to register with server\n");
        return -1;
    }

    printf("Sent registration message to server, waiting for response...\n");

    ServerMessage response;
    time_t        start_time = time(NULL);
    int           registration_received = 0;
    int           is_main_received = 0;

    while (time(NULL) - start_time < UDP_REGISTRATION_TIMEOUT) {
        if (sr_receive_udp_server_message(client, &response) == 0) {
            printf("Received message type: %d from server\n", response.type);

            if (response.type == SERVER_MSG_REGISTER_CONFIRM) {
                if (response.client_id >= 0) {
                    client->client_id = response.client_id;
                    client->registered = 1;
                    client->is_main = 0;
                    printf(
                        "Successfully registered with server, client ID: %d\n",
                        client->client_id
                    );
                    registration_received = 1;
                } else {
                    printf("Server rejected registration\n");
                    return -1;
                }
            } else if (response.type == SERVER_MSG_IS_MAIN) {
                printf("Received IS_MAIN message - this is the main client!\n");
                is_main_received = 1;
                client->is_main = 1;

                if (registration_received) {
                    return 0;
                }
            }
        }
        usleep(100000);
    }

    if (!registration_received) {
        printf("Registration timeout - no response from server\n");
        return -1;
    }

    if (registration_received && !is_main_received) {
        printf("Registered successfully but not the main client\n");
        return 0;
    }

    return 0;
}

void sr_udp_client_close(UDPClient *client) {
    if (client->registered) {
        ClientMessage disconnect_msg = { .type = CLIENT_MSG_DISCONNECT,
                                         .timestamp = time(NULL) };
        sr_send_udp_message_to_server(client, &disconnect_msg);
    }
    close(client->server_fd);
}

int sr_receive_udp_server_message(UDPClient *client, ServerMessage *msg) {
    if (!client || !msg) {
        return -1;
    }

    struct sockaddr_in from_addr;
    socklen_t          from_len = sizeof(from_addr);

    ssize_t bytes_read = recvfrom(
        client->server_fd,
        msg,
        sizeof(ServerMessage),
        0,
        (struct sockaddr *)&from_addr,
        &from_len
    );

    if (bytes_read == sizeof(ServerMessage)) {
        return 0;
    } else if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -3;
        }
        return -2;
    } else {
        return -2;
    }
}

int sr_send_udp_message_to_server(UDPClient *client, const ClientMessage *msg) {
    if (!client || !msg) {
        return -1;
    }

    ssize_t bytes_sent = sendto(
        client->server_fd,
        msg,
        sizeof(ClientMessage),
        0,
        (struct sockaddr *)&client->server_addr,
        client->addr_len
    );

    if (bytes_sent != sizeof(ClientMessage)) {
        perror("ERROR in sr_send_udp_message_to_server");
        return -2;
    }

    return 0;
}
