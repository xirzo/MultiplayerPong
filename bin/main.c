#include "aabb.h"
#include "flecs.h"
#include "input.h"
#include "movement.h"
#include "properties.h"
#include "raylib.h"
#include "render.h"
#include "score.h"
#include "server.h"
#include <stdio.h>
#include <wchar.h>

#define DEBUGGING

#define UP_DIRECTION -1.f
#define DOWN_DIRECTION 1.f

// TODO: remove global state
static bool is_main = false;

typedef struct PlayerInput {
  InputAction up_action;
  InputAction down_action;
  Input *input;
} PlayerInput;

void HandlePlayerInputActions(ecs_iter_t *it) {
  PaddleMovement *movements = ecs_field(it, PaddleMovement, 0);
  PlayerInput *inputs = ecs_field(it, PlayerInput, 1);

  for (size_t i = 0; i < it->count; i++) {
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
  const Properties *properties;
} MoveEnemyContext;

void MoveEnemy(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);

  MoveEnemyContext *ctx = (MoveEnemyContext *)it->param;

  Client *client = ctx->client;
  const Properties *properties = ctx->properties;

  for (size_t i = 0; i < it->count; i++) {
    Position *pos = &positions[i];

    ServerMessage response;

    if (sr_receive_server_message(client, &response) == 0) {
      switch (response.type) {
      case SERVER_MSG_PADDLE_POSITION_UPDATE: {
        pos->x =
            properties->SCREEN_WIDTH - properties->PADDLE_SCREEN_SIZE_MARGIN;
        pos->y = response.data.position.y;
        break;
      }
      default: {
        break;
      }
      }
    }
  }
}

typedef struct {
  Client *client;
  const Properties *properties;
  ecs_entity_t ball_entity;
} NetworkContext;

void UpdateBallFromNetwork(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);

  NetworkContext *ctx = (NetworkContext *)it->param;
  Client *client = ctx->client;

  for (size_t i = 0; i < it->count; i++) {
    Position *pos = &positions[i];

    ServerMessage response;
    while (sr_receive_server_message(client, &response) == 0) {
      switch (response.type) {
      case SERVER_MSG_BALL_POSITION_UPDATE:
        printf("Non-main client: Updated ball position to (%.2f, %.2f)\n",
               response.data.position.x, response.data.position.y);
        pos->x = response.data.position.x;
        pos->y = response.data.position.y;
        break;
      default:
        break;
      }
    }
  }
}

int main(void) {
  const Properties properties = {
      .SCREEN_WIDTH = 600,
      .SCREEN_HEIGHT = 300,
      .FPS_LOCK = 60,
      .PADDLE_SPEED = 450.f,
      .PADDLE_WIDTH = 20.f,
      .PADDLE_HEIGHT = 80.f,
      .PADDLE_SCREEN_SIZE_MARGIN = 50.f,
      .BALL_SIDE = 10.f,
      .BALL_MIN_SPEED = 450.f,
      .BALL_MAX_SPEED = 650.f,
      .BALL_INITIAL_DIRECTION = {0.7f, -0.7f},
      .WALL_THICKNESS = 10.f,
      .MIDDLE_LINE_WIDTH = 10.f,
      .SERVER_IP = "127.0.0.1",
      .SERVER_PORT = 8080,
  };

  Client *client = malloc(sizeof(Client));

  if (!client) {
    fprintf(stderr, "error: Failed to allocate memory for client\n");
    return 1;
  }

  if ((sr_client_connect(client, properties.SERVER_IP,
                         properties.SERVER_PORT)) != 0) {
    fprintf(stderr, "error: Failed to connect to the server\n");
    fprintf(stderr, "Make sure the server is running on %s:%d\n",
            properties.SERVER_IP, properties.SERVER_PORT);
    free(client);
    return 1;
  }

  MoveEnemyContext ctx = {.client = client, .properties = &properties};

  InitWindow(properties.SCREEN_WIDTH, properties.SCREEN_HEIGHT, "game");
  SetTargetFPS(properties.FPS_LOCK);

  ecs_world_t *world = ecs_init();

  ECS_COMPONENT(world, Position);
  ECS_COMPONENT(world, BallMovement);
  ECS_COMPONENT(world, PaddleMovement);
  ECS_COMPONENT(world, MovementClamp);
  ECS_COMPONENT(world, RenderableRectangle);
  ECS_COMPONENT(world, PlayerInput);
  ECS_COMPONENT(world, Collider);
  ECS_COMPONENT(world, Score);

  ECS_TAG(world, Ball);
  // TODO: rename Paddle to Player
  ECS_TAG(world, Paddle);
  ECS_TAG(world, Enemy);
  ECS_TAG(world, Wall);

  ECS_SYSTEM(world, BallScoringSystem,
             EcsOnUpdate, [out] Position, [inout] BallMovement, Ball);
  ECS_SYSTEM(world, MoveBall, EcsOnUpdate, [out] Position, [inout] BallMovement,
             Ball);
  ECS_SYSTEM(world, MovePaddle,
             EcsOnUpdate, [out] Position, [inout] PaddleMovement, Paddle);
  ECS_SYSTEM(world, ClampPosition, EcsOnUpdate, [out] Position, MovementClamp);
  ECS_SYSTEM(world, RenderRectangle,
             EcsOnUpdate, [in] Position, [in] RenderableRectangle);
  ECS_SYSTEM(world, HandlePlayerInputActions,
             EcsOnUpdate, [out] PaddleMovement, [in] PlayerInput);
  ECS_SYSTEM(world, BallPaddleCollisions, EcsOnUpdate,
             Position, [inout] BallMovement, [in] Collider, Ball);
  ECS_SYSTEM(world, BallWallCollisions, EcsOnUpdate,
             Position, [inout] BallMovement, [in] Collider, Ball);
  ECS_SYSTEM(world, MoveEnemy, EcsOnUpdate, [out] Position, Enemy);
  ECS_SYSTEM(world, UpdateBallFromNetwork, EcsOnUpdate, [out] Position, Ball);

  Input left_paddle_input;
  InitInput(&left_paddle_input);
  left_paddle_input.bindings[ACTION_MOVE_UP].key = KEY_K;
  left_paddle_input.bindings[ACTION_MOVE_DOWN].key = KEY_J;

  ecs_entity_t player =
      ecs_insert(world,
                 ecs_value(Position, {properties.PADDLE_SCREEN_SIZE_MARGIN,
                                      (float)properties.SCREEN_HEIGHT / 2 -
                                          properties.PADDLE_HEIGHT / 2}),
                 ecs_value(PaddleMovement,
                           {
                               .speed = properties.PADDLE_SPEED,
                               .direction = {0, 0},
                               .velocity = {0, 0},
                           }),
                 ecs_value(RenderableRectangle,
                           {
                               .width = properties.PADDLE_WIDTH,
                               .height = properties.PADDLE_HEIGHT,
                               .color = RED,
                           }),
                 ecs_value(PlayerInput,
                           {
                               .up_action = ACTION_MOVE_UP,
                               .down_action = ACTION_MOVE_DOWN,
                               .input = &left_paddle_input,
                           }),
                 ecs_value(Collider,
                           {
                               .width = properties.PADDLE_WIDTH,
                               .height = properties.PADDLE_HEIGHT,
                           }),
                 ecs_value(MovementClamp,
                           {
                               .lower_limit = 0,
                               .upper_limit = properties.SCREEN_HEIGHT -
                                              properties.PADDLE_HEIGHT,
                           }),
                 ecs_value(Score, {.value = 0}));

  ecs_add_id(world, player, Paddle);

  ecs_entity_t right_paddle =
      ecs_insert(world,
                 ecs_value(Position, {properties.SCREEN_WIDTH -
                                          properties.PADDLE_SCREEN_SIZE_MARGIN -
                                          properties.PADDLE_WIDTH,
                                      (float)properties.SCREEN_HEIGHT / 2 -
                                          properties.PADDLE_HEIGHT / 2}),
                 ecs_value(RenderableRectangle,
                           {
                               .width = properties.PADDLE_WIDTH,
                               .height = properties.PADDLE_HEIGHT,
                               .color = GREEN,
                           }),
                 ecs_value(Collider,
                           {
                               .width = properties.PADDLE_WIDTH,
                               .height = properties.PADDLE_HEIGHT,
                           }),
                 ecs_value(Score, {.value = 0}));

  ecs_add_id(world, right_paddle, Enemy);

  ecs_entity_t ball = ecs_insert(
      world,
      ecs_value(Position, {.x = (float)properties.SCREEN_WIDTH / 2,
                           .y = (float)properties.SCREEN_HEIGHT / 2}),
      ecs_value(BallMovement,
                {
                    .current_speed = properties.BALL_MIN_SPEED,
                    .min_speed = properties.BALL_MIN_SPEED,
                    .max_speed = properties.BALL_MAX_SPEED,
                    .direction = properties.BALL_INITIAL_DIRECTION,
                    .velocity = {0, 0},
                }),
      ecs_value(RenderableRectangle,
                {properties.BALL_SIDE, properties.BALL_SIDE, WHITE}),
      ecs_value(Collider, {properties.BALL_SIDE, properties.BALL_SIDE}));

  ecs_add_id(world, ball, Ball);

  NetworkContext network_ctx = {
      .client = client, .properties = &properties, .ball_entity = ball};

  ecs_entity_t upper_wall =
      ecs_insert(world, ecs_value(Position, {.x = 0, .y = 0}),
                 ecs_value(Collider, {properties.SCREEN_WIDTH,
                                      properties.WALL_THICKNESS}));

  ecs_entity_t lower_wall = ecs_insert(
      world, ecs_value(Position, {.x = 0, .y = properties.SCREEN_HEIGHT}),
      ecs_value(Collider,
                {properties.SCREEN_WIDTH, properties.WALL_THICKNESS}));

  ecs_add_id(world, upper_wall, Wall);
  ecs_add_id(world, lower_wall, Wall);

  while (!WindowShouldClose()) {
    ServerMessage msg;

    while (sr_receive_server_message(client, &msg) == 0) {
      switch (msg.type) {
      case SERVER_MSG_IS_MAIN:
        printf("Current client is main\n");
        is_main = true;
        break;
      case SERVER_MSG_BALL_POSITION_UPDATE:
        if (!is_main) {
          printf("Received ball position update: (%.2f, %.2f)\n",
                 msg.data.position.x, msg.data.position.y);
          ecs_set(world, ball, Position,
                  {.x = msg.data.position.x, .y = msg.data.position.y});
        }
        break;
      default:
        break;
      }
    }

    UpdateInput(&left_paddle_input);

    ecs_run(world, ecs_id(ClampPosition), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(HandlePlayerInputActions), GetFrameTime(),
            (void *)&properties);
    ecs_run(world, ecs_id(MovePaddle), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(MoveEnemy), GetFrameTime(), (void *)&ctx);

    if (is_main) {
      ecs_run(world, ecs_id(BallScoringSystem), GetFrameTime(),
              (void *)&properties);
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
      ecs_run(world, ecs_id(UpdateBallFromNetwork), GetFrameTime(),
              (void *)&network_ctx);
    }

    // TODO: add system for player_pos sending
    const Position *player_pos = ecs_get(world, player, Position);
    sr_send_position_to_server(client, (vec2){player_pos->x, player_pos->y});

    BeginDrawing();
    ClearBackground(BLACK);

    DrawRectangle(properties.SCREEN_WIDTH / 2, 0, properties.MIDDLE_LINE_WIDTH,
                  properties.SCREEN_HEIGHT, WHITE);

#ifdef DEBUGGING
    const Position *ball_pos = ecs_get(world, ball, Position);
    const Score *left_score = ecs_get(world, player, Score);
    const Score *right_score = ecs_get(world, right_paddle, Score);

    DrawText(
        TextFormat("Ball Position: x: %.1f, y: %.1f", ball_pos->x, ball_pos->y),
        10.f, 15.f, 20.f, WHITE);

    if (is_main) {

      DrawText(TextFormat("Left score: %d", left_score->value), 10.f, 35.f,
               20.f, WHITE);

      DrawText(TextFormat("Right score: %d", right_score->value), 10.f, 55.f,
               20.f, WHITE);
    }

    DrawFPS(10.f, properties.SCREEN_HEIGHT - 30.f);
#endif

    ecs_run(world, ecs_id(RenderRectangle), GetFrameTime(), NULL);
    EndDrawing();
  }

  sr_client_close(client);
  free(client);

  CloseWindow();
  ecs_fini(world);
  return 0;
}
