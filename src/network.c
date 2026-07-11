#include "network.h"

#include <stdio.h>

#include "components.h"
#include "movement.h"
#include "properties.h"

void HandleServerMessage(GameState* state, const ServerMessage* msg) {
  switch (msg->type) {
    case SERVER_MSG_BALL_POSITION_UPDATE:
      if (!state->client->is_main) {
        float mirrored_x = g_Properties.SCREEN_WIDTH - msg->data.position.x;
        float mirrored_y = msg->data.position.y;
        ecs_set(state->world, state->ball_entity, Position,
                {.x = mirrored_x, .y = mirrored_y});
      }
      break;

    case SERVER_MSG_PADDLE_POSITION_UPDATE:
      if (state->client->is_main) {
        ecs_set(state->world, state->enemy_entity, Position,
                {.x = g_Properties.SCREEN_WIDTH -
                      g_Properties.PADDLE_SCREEN_SIZE_MARGIN -
                      g_Properties.PADDLE_WIDTH,
                 .y = msg->data.position.y});
        printf("Main client: Updated enemy paddle on right\n");
      } else {
        float mirrored_x = g_Properties.SCREEN_WIDTH - msg->data.position.x -
                           g_Properties.PADDLE_WIDTH;
        ecs_set(state->world, state->enemy_entity, Position,
                {.x = mirrored_x, .y = msg->data.position.y});
        printf("Non-main: Set enemy paddle on right at (%.2f, %.2f)\n",
               mirrored_x, msg->data.position.y);
      }
      break;

    default:
      break;
  }
}
