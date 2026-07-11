#include "network_systems.h"

#include <stdio.h>

#include "components.h"
#include "input.h"
#include "movement.h"
#include "player.h"
#include "properties.h"
#include "server.h"

#define UP_DIRECTION -1.0f
#define DOWN_DIRECTION 1.0f

void ProcessInputSystem(ecs_iter_t* it) {
  PaddleMovement* movements = ecs_field(it, PaddleMovement, 0);
  PlayerInput* inputs = ecs_field(it, PlayerInput, 1);

  for (int i = 0; i < it->count; i++) {
    movements[i].direction.y = 0;

    Input* input = inputs[i].input;

    if (IsActionPressed(input, inputs[i].up_action)) {
      movements[i].direction.y = UP_DIRECTION;
    }
    if (IsActionPressed(input, inputs[i].down_action)) {
      movements[i].direction.y = DOWN_DIRECTION;
    }
  }
}

void MoveEnemySystem(ecs_iter_t* it) {
  Position* positions = ecs_field(it, Position, 0);
  UDPClient* client = (UDPClient*)it->param;

  for (int i = 0; i < it->count; i++) {
    Position* pos = &positions[i];

    ServerMessage response;
    if (sr_receive_udp_server_message(client, &response) == 0) {
      switch (response.type) {
        case SERVER_MSG_PADDLE_POSITION_UPDATE: {
          printf("Received paddle update from client %d: (%.2f, %.2f)\n",
                 response.client_id, response.data.position.x,
                 response.data.position.y);

          if (client->is_main) {
            pos->x = g_Properties.SCREEN_WIDTH -
                     g_Properties.PADDLE_SCREEN_SIZE_MARGIN -
                     g_Properties.PADDLE_WIDTH;
            pos->y = response.data.position.y;
            printf("Main client: Set enemy paddle to right side (%.2f,%.2f)\n",
                   pos->x, pos->y);
          } else {
            float mirrored_x = g_Properties.SCREEN_WIDTH -
                               response.data.position.x -
                               g_Properties.PADDLE_WIDTH;
            pos->x = mirrored_x;
            pos->y = response.data.position.y;
            printf("Non-main client: Mirrored enemy paddle from (%.2f, %.2f) "
                   "to (%.2f, %.2f)\n",
                   response.data.position.x, response.data.position.y,
                   pos->x, pos->y);
          }
          break;
        }
        default:
          break;
      }
    }
  }
}

void UpdateBallFromNetwork(ecs_iter_t* it) {
  Position* positions = ecs_field(it, Position, 0);
  UDPClient* client = (UDPClient*)it->param;

  for (int i = 0; i < it->count; i++) {
    Position* pos = &positions[i];

    ServerMessage response;
    while (sr_receive_udp_server_message(client, &response) == 0) {
      switch (response.type) {
        case SERVER_MSG_BALL_POSITION_UPDATE: {
          float mirrored_x =
              g_Properties.SCREEN_WIDTH - response.data.position.x;
          float mirrored_y = response.data.position.y;

          printf("Non-main client: Ball position (%.2f, %.2f) -> mirrored "
                 "(%.2f, %.2f)\n",
                 response.data.position.x, response.data.position.y,
                 mirrored_x, mirrored_y);

          pos->x = mirrored_x;
          pos->y = mirrored_y;
          break;
        }
        default:
          break;
      }
    }
  }
}

void SendPlayerPositionSystem(ecs_iter_t* it) {
  Position* positions = ecs_field(it, Position, 0);
  UDPClient* client = (UDPClient*)it->param;

  for (int i = 0; i < it->count; i++) {
    const ClientMessage msg = {
        .type = CLIENT_MSG_PADDLE_POSITION,
        .data.position = {positions[i].x, positions[i].y},
    };
    sr_send_udp_message_to_server(client, &msg);
  }
}
