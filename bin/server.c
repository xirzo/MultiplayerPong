#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "properties.h"

#include "server.h"

int main(void) {
    if (properties_load_from_file("configuration.game")) {
        printf("Succesfully loaded properties from configuration file\n");
        printf(
            "Server: %s:%d\n", g_Properties.SERVER_IP, g_Properties.SERVER_PORT
        );
    } else {
        printf("Using default settings\n");
    }

    Server *server =
        sr_create_server(g_Properties.SERVER_IP, g_Properties.SERVER_PORT);

    if (!server) {
        fprintf(stderr, "error: Failed to create server\n");
        return 1;
    }

    sr_start_listen(server);

    sr_destroy_server(server);
    return 0;
}
