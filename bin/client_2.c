#include "server.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void handle_server_message(const ServerMessage *msg);

int main() {
  Client *client = malloc(sizeof(Client));

  if (sr_client_connect(client, "127.0.0.1", 8080) != 0) {
    printf("Failed to connect to server\n");
    free(client);
    return -1;
  }

  for (float i = 0; i < 3.f; i++) {
    vec2 pos = {0.f + i, 0.f};

    sr_send_position_to_server(client, pos);

    printf("Sent position (%.2f, %.2f)\n", pos.x, pos.y);

    ServerMessage response;

    if (sr_receive_server_message(client, &response) == 0) {
      handle_server_message(&response);
    }

    usleep(100000);
  }

  printf("Listening for more server messages...\n");

  for (int i = 0; i < 10; i++) {
    ServerMessage response;

    if (sr_receive_server_message(client, &response) == 0) {
      handle_server_message(&response);
    }

    usleep(500000);
  }

  sr_client_close(client);
  free(client);
  return 0;
}

void handle_server_message(const ServerMessage *msg) {
  switch (msg->type) {
  case SERVER_MSG_POSITION_UPDATE:
    printf("Player %d moved to (%.2f, %.2f)\n", msg->client_id,
           msg->data.position.x, msg->data.position.y);
    break;

  case SERVER_MSG_PLAYER_JOINED:
    printf("Player %d joined: %s\n", msg->data.player_info.player_id,
           msg->data.player_info.player_name);
    break;

  case SERVER_MSG_PLAYER_LEFT:
    printf("Player %d left the game\n", msg->client_id);
    break;

  default:
    printf("Unknown message type: %d\n", msg->type);
    break;
  }
}
