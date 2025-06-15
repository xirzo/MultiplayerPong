#include "aabb.h"
#include "flecs.h"
#include "input.h"
#include "movement.h"
#include "properties.h"
#include "raylib.h"
#include "render.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUGGING

#define UP_DIRECTION -1.f
#define DOWN_DIRECTION 1.f

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

int main(void) {
  const Properties properties = {
      .SCREEN_WIDTH = 1280,
      .SCREEN_HEIGHT = 720,
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
  };

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

  ECS_TAG(world, Ball);
  ECS_TAG(world, Paddle);
  ECS_TAG(world, Wall);

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

  Input left_paddle_input;
  InitInput(&left_paddle_input);
  left_paddle_input.bindings[ACTION_MOVE_UP].key = KEY_W;
  left_paddle_input.bindings[ACTION_MOVE_DOWN].key = KEY_S;

  Input right_paddle_input;
  InitInput(&right_paddle_input);
  right_paddle_input.bindings[ACTION_MOVE_UP].key = KEY_UP;
  right_paddle_input.bindings[ACTION_MOVE_DOWN].key = KEY_DOWN;

  ecs_entity_t left_paddle = ecs_insert(
      world,
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
      ecs_value(MovementClamp, {
                                   .lower_limit = 0,
                                   .upper_limit = properties.SCREEN_HEIGHT -
                                                  properties.PADDLE_HEIGHT,
                               }));

  ecs_add_id(world, left_paddle, Paddle);

  ecs_entity_t right_paddle = ecs_insert(
      world,
      ecs_value(
          Position,
          {properties.SCREEN_WIDTH - properties.PADDLE_SCREEN_SIZE_MARGIN -
               properties.PADDLE_WIDTH,
           (float)properties.SCREEN_HEIGHT / 2 - properties.PADDLE_HEIGHT / 2}),
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
                    .color = GREEN,
                }),
      ecs_value(PlayerInput,
                {
                    .up_action = ACTION_MOVE_UP,
                    .down_action = ACTION_MOVE_DOWN,
                    .input = &right_paddle_input,
                }),
      ecs_value(Collider,
                {
                    .width = properties.PADDLE_WIDTH,
                    .height = properties.PADDLE_HEIGHT,
                }),
      ecs_value(MovementClamp, {
                                   .lower_limit = 0,
                                   .upper_limit = properties.SCREEN_HEIGHT -
                                                  properties.PADDLE_HEIGHT,
                               }));

  ecs_add_id(world, right_paddle, Paddle);

  ecs_entity_t ball = ecs_insert(
      world,
      ecs_value(Position, {.x = (float)properties.SCREEN_WIDTH / 2,
                           .y = (float)properties.SCREEN_HEIGHT / 2}),
      ecs_value(BallMovement,
                {
                    .min_speed = properties.BALL_MIN_SPEED,
                    .max_speed = properties.BALL_MAX_SPEED,
                    .direction = properties.BALL_INITIAL_DIRECTION,
                    .velocity = {0, 0},
                }),
      ecs_value(RenderableRectangle,
                {properties.BALL_SIDE, properties.BALL_SIDE, WHITE}),
      ecs_value(Collider, {properties.BALL_SIDE, properties.BALL_SIDE}));

  ecs_add_id(world, ball, Ball);

  ecs_entity_t upper_wall =
      ecs_insert(world, ecs_value(Position, {.x = 0, .y = 0}),
                 ecs_value(Collider, {properties.SCREEN_WIDTH,
                                      properties.WALL_THICKNESS}));

  ecs_entity_t lower_wall = ecs_insert(
      world,
      ecs_value(
          Position,
          {.x = 0, .y = properties.SCREEN_HEIGHT - properties.PADDLE_HEIGHT}),
      ecs_value(Collider,
                {properties.SCREEN_WIDTH, properties.WALL_THICKNESS}));

  ecs_add_id(world, upper_wall, Wall);
  ecs_add_id(world, lower_wall, Wall);

  while (!WindowShouldClose()) {
    UpdateInput(&left_paddle_input);
    UpdateInput(&right_paddle_input);

    ecs_run(world, ecs_id(ClampPosition), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(BallPaddleCollisions), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(BallWallCollisions), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(HandlePlayerInputActions), GetFrameTime(),
            (void *)&properties);
    ecs_run(world, ecs_id(MoveBall), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(MovePaddle), GetFrameTime(), NULL);

    BeginDrawing();
    ClearBackground(BLACK);

#ifdef DEBUGGING
    const Position *ball_pos = ecs_get(world, ball, Position);

    DrawText(
        TextFormat("Ball Position: x: %.1f, y: %.1f", ball_pos->x, ball_pos->y),
        10.f, 15.f, 20.f, WHITE);

    DrawFPS(10.f, properties.SCREEN_HEIGHT - 30.f);
#endif

    ecs_run(world, ecs_id(RenderRectangle), GetFrameTime(), NULL);
    EndDrawing();
  }

  CloseWindow();
  ecs_fini(world);
  return 0;
}

// TODO: add score
