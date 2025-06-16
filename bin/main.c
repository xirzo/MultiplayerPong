#include "aabb.h"
#include "flecs.h"
#include "input.h"
#include "movement.h"
#include "player.h"
#include "properties.h"
#include "raylib.h"
#include "render.h"
#include "score.h"
#include "server.h"
#include <stdio.h>
#include <wchar.h>

// #define DEBUGGING

#define UP_DIRECTION -1.f
#define DOWN_DIRECTION 1.f

// TODO: remove global state
static bool is_main = false;

typedef struct PlayerInput {
  InputAction up_action;
  InputAction down_action;
  Input *input;
} PlayerInput;

void ProcessInputSystem(ecs_iter_t *it) {
  PaddleMovement *movements = ecs_field(it, PaddleMovement, 0);
  PlayerInput *inputs = ecs_field(it, PlayerInput, 1);

  for (int i = 0; i < it->count; i++) {
    movements[i].direction.y = 0;

    Input *input = inputs[i].input;

    if (IsActionPressed(input, inputs[i].up_action)) {
      movements[i].direction.y = UP_DIRECTION;
    }
    if (IsActionPressed(input, inputs[i].down_action)) {
      movements[i].direction.y = DOWN_DIRECTION;
    }
  }
}

// TODO: remove context resolve this via new system
typedef struct {
  Client *client;
} MoveEnemyContext;

void MoveEnemySystem(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);

  Client *client = (Client *)it->param;

  for (int i = 0; i < it->count; i++) {
    Position *pos = &positions[i];

    ServerMessage response;
    if (sr_receive_server_message(client, &response) == 0) {
      switch (response.type) {
      case SERVER_MSG_PADDLE_POSITION_UPDATE: {
        printf("Received paddle update from client %d: (%.2f, %.2f)\n",
               response.client_id, response.data.position.x,
               response.data.position.y);

        if (is_main) {
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
          printf("Non-main client: Mirrored enemy paddle from (%.2f, %.2f) to "
                 "(%.2f, %.2f)\n",
                 response.data.position.x, response.data.position.y, pos->x,
                 pos->y);
        }
        break;
      }
      default:
        break;
      }
    }
  }
}

typedef struct {
  Client *client;
  ecs_entity_t ball_entity;
} NetworkContext;

void UpdateBallFromNetwork(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);

  Client *client = (Client *)it->param;

  for (int i = 0; i < it->count; i++) {
    Position *pos = &positions[i];

    ServerMessage response;

    while (sr_receive_server_message(client, &response) == 0) {
      switch (response.type) {
      case SERVER_MSG_BALL_POSITION_UPDATE: {
        float mirrored_x = g_Properties.SCREEN_WIDTH - response.data.position.x;
        float mirrored_y = response.data.position.y;

        printf("Non-main client: Ball position (%.2f, %.2f) -> mirrored (%.2f, "
               "%.2f)\n",
               response.data.position.x, response.data.position.y, mirrored_x,
               mirrored_y);

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

int main(void) {
  Client *client = malloc(sizeof(Client));

  if (!client) {
    fprintf(stderr, "error: Failed to allocate memory for client\n");
    return 1;
  }

  if ((sr_client_connect(client, g_Properties.SERVER_IP,
                         g_Properties.SERVER_PORT)) != 0) {
    fprintf(stderr, "error: Failed to connect to the server\n");
    fprintf(stderr, "Make sure the server is running on %s:%d\n",
            g_Properties.SERVER_IP, g_Properties.SERVER_PORT);
    free(client);
    return 1;
  }

  InitWindow(g_Properties.SCREEN_WIDTH, g_Properties.SCREEN_HEIGHT, "game");
  SetTargetFPS(g_Properties.FPS_LOCK);

  ecs_world_t *world = ecs_init();

  ECS_COMPONENT(world, Position);
  ECS_COMPONENT(world, BallMovement);
  ECS_COMPONENT(world, PaddleMovement);
  ECS_COMPONENT(world, MovementClamp);
  ECS_COMPONENT(world, RenderableRectangle);
  ECS_COMPONENT(world, PlayerInput);
  ECS_COMPONENT(world, Collider);
  ECS_COMPONENT(world, Score);

  ECS_COMPONENT(world, Player);

  ECS_TAG(world, Ball);
  // TODO: rename Paddle to Player
  // TODO: solve problem of having 3 tags
  ECS_TAG(world, Paddle);
  ECS_TAG(world, Enemy);
  ECS_TAG(world, Wall);

  ECS_SYSTEM(world, ScoreCountSystem,
             EcsOnUpdate, [out] Position, [inout] BallMovement, Ball);
  ECS_SYSTEM(world, MoveBall, EcsOnUpdate, [out] Position, [inout] BallMovement,
             Ball);
  ECS_SYSTEM(world, MovePlayerSystem,
             EcsOnUpdate, [out] Position, [inout] PaddleMovement, Paddle);
  ECS_SYSTEM(world, ClampMovementSystem, EcsOnUpdate, [out] Position,
             MovementClamp);
  ECS_SYSTEM(world, RenderRectangle,
             EcsOnUpdate, [in] Position, [in] RenderableRectangle);
  ECS_SYSTEM(world, ProcessInputSystem,
             EcsOnUpdate, [out] PaddleMovement, [in] PlayerInput);
  ECS_SYSTEM(world, BallPaddleCollisions, EcsOnUpdate,
             Position, [inout] BallMovement, [in] Collider, Ball);
  ECS_SYSTEM(world, BallWallCollisions, EcsOnUpdate,
             Position, [inout] BallMovement, [in] Collider, Ball);
  ECS_SYSTEM(world, MoveEnemySystem, EcsOnUpdate, [out] Position, Enemy);
  ECS_SYSTEM(world, UpdateBallFromNetwork, EcsOnUpdate, [out] Position, Ball);

  Input left_paddle_input;
  InitInput(&left_paddle_input);
  left_paddle_input.bindings[ACTION_MOVE_UP].key = KEY_K;
  left_paddle_input.bindings[ACTION_MOVE_DOWN].key = KEY_J;

  ecs_entity_t player =
      ecs_insert(world,
                 ecs_value(Position, {g_Properties.PADDLE_SCREEN_SIZE_MARGIN,
                                      (float)g_Properties.SCREEN_HEIGHT / 2 -
                                          g_Properties.PADDLE_HEIGHT / 2}),
                 ecs_value(PaddleMovement,
                           {
                               .speed = g_Properties.PADDLE_SPEED,
                               .direction = {0, 0},
                               .velocity = {0, 0},
                           }),
                 ecs_value(RenderableRectangle,
                           {
                               .width = g_Properties.PADDLE_WIDTH,
                               .height = g_Properties.PADDLE_HEIGHT,
                               .color = RED,
                           }),
                 ecs_value(PlayerInput,
                           {
                               .up_action = ACTION_MOVE_UP,
                               .down_action = ACTION_MOVE_DOWN,
                               .input = &left_paddle_input,
                           }),

                 ecs_value(Player,
                           {
                               .is_main = 0,
                           }),
                 ecs_value(Collider,
                           {
                               .width = g_Properties.PADDLE_WIDTH,
                               .height = g_Properties.PADDLE_HEIGHT,
                           }),
                 ecs_value(MovementClamp,
                           {
                               .lower_limit = 0,
                               .upper_limit = g_Properties.SCREEN_HEIGHT -
                                              g_Properties.PADDLE_HEIGHT,
                           }),
                 ecs_value(Score, {.value = 0}));

  ecs_add_id(world, player, Paddle);

  ecs_entity_t enemy = ecs_insert(
      world,
      ecs_value(Position, {g_Properties.SCREEN_WIDTH -
                               g_Properties.PADDLE_SCREEN_SIZE_MARGIN -
                               g_Properties.PADDLE_WIDTH,
                           (float)g_Properties.SCREEN_HEIGHT / 2 -
                               g_Properties.PADDLE_HEIGHT / 2}),
      ecs_value(RenderableRectangle,
                {
                    .width = g_Properties.PADDLE_WIDTH,
                    .height = g_Properties.PADDLE_HEIGHT,
                    .color = GREEN,
                }),
      ecs_value(Collider,
                {
                    .width = g_Properties.PADDLE_WIDTH,
                    .height = g_Properties.PADDLE_HEIGHT,
                }),
      ecs_value(Score, {.value = 0}));

  ecs_add_id(world, enemy, Enemy);
  ecs_add_id(world, enemy, Paddle);

  ecs_entity_t ball = ecs_insert(
      world,
      ecs_value(Position, {.x = (float)g_Properties.SCREEN_WIDTH / 2,
                           .y = (float)g_Properties.SCREEN_HEIGHT / 2}),
      ecs_value(BallMovement,
                {
                    .current_speed = g_Properties.BALL_MIN_SPEED,
                    .min_speed = g_Properties.BALL_MIN_SPEED,
                    .max_speed = g_Properties.BALL_MAX_SPEED,
                    .direction = g_Properties.BALL_INITIAL_DIRECTION,
                    .velocity = {0, 0},
                }),
      ecs_value(RenderableRectangle,
                {g_Properties.BALL_SIDE, g_Properties.BALL_SIDE, WHITE}),
      ecs_value(Collider, {g_Properties.BALL_SIDE, g_Properties.BALL_SIDE}));

  ecs_add_id(world, ball, Ball);

  ecs_entity_t upper_wall =
      ecs_insert(world, ecs_value(Position, {.x = 0, .y = 0}),
                 ecs_value(Collider, {g_Properties.SCREEN_WIDTH,
                                      g_Properties.WALL_THICKNESS}));

  ecs_add_id(world, upper_wall, Wall);

  ecs_entity_t lower_wall = ecs_insert(
      world, ecs_value(Position, {.x = 0, .y = g_Properties.SCREEN_HEIGHT}),
      ecs_value(Collider,
                {g_Properties.SCREEN_WIDTH, g_Properties.WALL_THICKNESS}));

  ecs_add_id(world, lower_wall, Wall);

  while (!WindowShouldClose()) {
    ServerMessage msg;

    while (sr_receive_server_message(client, &msg) == 0) {
      switch (msg.type) {
      case SERVER_MSG_IS_MAIN: {
        is_main = true;

        // FIX: move this into MoveEnemySystem (why does this move player)
        // ecs_set(world, player, Position,
        //         {.x = g_Properties.PADDLE_SCREEN_SIZE_MARGIN,
        //          .y = (float)g_Properties.SCREEN_HEIGHT / 2 -
        //               g_Properties.PADDLE_HEIGHT / 2});

        // ecs_set(world, enemy, Position,
        //         {.x = g_Properties.SCREEN_WIDTH -
        //               g_Properties.PADDLE_SCREEN_SIZE_MARGIN -
        //               g_Properties.PADDLE_WIDTH,
        //          .y = (float)g_Properties.SCREEN_HEIGHT / 2 -
        //               g_Properties.PADDLE_HEIGHT / 2});
        break;
      }

      case SERVER_MSG_BALL_POSITION_UPDATE: {
        if (!is_main) {
          float mirrored_x = g_Properties.SCREEN_WIDTH - msg.data.position.x;
          float mirrored_y = msg.data.position.y;
          ecs_set(world, ball, Position, {.x = mirrored_x, .y = mirrored_y});
        }
        break;
      }

      case SERVER_MSG_PADDLE_POSITION_UPDATE: {
        if (is_main) {
          ecs_set(world, enemy, Position,
                  {.x = g_Properties.SCREEN_WIDTH -
                        g_Properties.PADDLE_SCREEN_SIZE_MARGIN -
                        g_Properties.PADDLE_WIDTH,
                   .y = msg.data.position.y});
          printf("Main client: Updated enemy paddle on right\n");
        } else {
          float mirrored_x = g_Properties.SCREEN_WIDTH - msg.data.position.x -
                             g_Properties.PADDLE_WIDTH;
          ecs_set(world, enemy, Position,
                  {.x = mirrored_x, .y = msg.data.position.y});
          printf("Non-main: Set enemy paddle on right at (%.2f, %.2f)\n",
                 mirrored_x, msg.data.position.y);
        }
        break;
      }

      default:
        break;
      }
    }

    UpdateInput(&left_paddle_input);

    ecs_run(world, ecs_id(ClampMovementSystem), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(ProcessInputSystem), GetFrameTime(),
            (void *)&g_Properties);
    ecs_run(world, ecs_id(MovePlayerSystem), GetFrameTime(), NULL);
    // ecs_run(world, ecs_id(MoveEnemySystem), GetFrameTime(), (void *)client);

    if (is_main) {
      ecs_run(world, ecs_id(ScoreCountSystem), GetFrameTime(),
              (void *)&g_Properties);
      ecs_run(world, ecs_id(BallPaddleCollisions), GetFrameTime(), NULL);
      ecs_run(world, ecs_id(BallWallCollisions), GetFrameTime(), NULL);
      ecs_run(world, ecs_id(MoveBall), GetFrameTime(), NULL);

      const Position *ball_pos = ecs_get(world, ball, Position);

      const ClientMessage ball_msg = {
          .type = CLIENT_MSG_BALL_POSITION,
          .data.position = {ball_pos->x, ball_pos->y},
      };

      sr_send_message_to_server(client, &ball_msg);
    } else {
      const Position *current_player_pos = ecs_get(world, player, Position);

      if (current_player_pos->x > (float)g_Properties.SCREEN_WIDTH / 2) {
        ecs_set(world, player, Position,
                {.x = g_Properties.PADDLE_SCREEN_SIZE_MARGIN,
                 .y = current_player_pos->y});
      }
    }

    // TODO: add system for player_pos sending

    ecs_query_t *player_q =
        ecs_query(world, {.terms = {{.id = ecs_id(Position), .inout = EcsIn},
                                    {.id = ecs_id(Player), .inout = EcsIn}},

                          .cache_kind = EcsQueryCacheAuto});

    ecs_iter_t it = ecs_query_iter(world, player_q);

    while (ecs_query_next(&it)) {
      Position *p = ecs_field(&it, Position, 0);

      for (int i = 0; i < it.count; i++) {
        sr_send_position_to_server(client, (vec2){p[i].x, p[i].y});
      }
    }

    ecs_query_fini(player_q);

    BeginDrawing();
    ClearBackground(BLACK);

    ecs_run(world, ecs_id(RenderRectangle), GetFrameTime(), NULL);

    DrawRectangle(g_Properties.SCREEN_WIDTH / 2, 0,
                  g_Properties.MIDDLE_LINE_WIDTH, g_Properties.SCREEN_HEIGHT,
                  WHITE);

#ifdef DEBUGGING
    const Position *ball_pos = ecs_get(world, ball, Position);
    const Score *left_score = ecs_get(world, player, Score);
    const Score *right_score = ecs_get(world, enemy, Score);

    DrawText(
        TextFormat("Ball Position: x: %.1f, y: %.1f", ball_pos->x, ball_pos->y),
        10.f, 15.f, 20.f, WHITE);

    if (is_main) {

      DrawText(TextFormat("Left score: %d", left_score->value), 10.f, 35.f,
               20.f, WHITE);

      DrawText(TextFormat("Right score: %d", right_score->value), 10.f, 55.f,
               20.f, WHITE);
    }

    DrawFPS(10.f, g_Properties.SCREEN_HEIGHT - 30.f);
#endif

    EndDrawing();
  }

  sr_client_close(client);
  free(client);

  CloseWindow();
  ecs_fini(world);
  return 0;
}
