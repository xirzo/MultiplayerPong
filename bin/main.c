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

#define UP_DIRECTION -1.f
#define DOWN_DIRECTION 1.f

typedef struct PlayerInput {
  InputAction up_action;
  InputAction down_action;
  Input *input;
} PlayerInput;

void HandlePlayerInputActions(ecs_iter_t *it) {
  Velocity *velocities = ecs_field(it, Velocity, 0);
  PlayerInput *inputs = ecs_field(it, PlayerInput, 1);

  Properties *properties = (Properties *)it->param;

  for (size_t i = 0; i < it->count; i++) {
    velocities[i].y = 0;
    Input *input = inputs[i].input;

    if (IsActionPressed(input, inputs[i].up_action)) {
      velocities[i].y = UP_DIRECTION * properties->PADDLE_SPEED;
    }
    if (IsActionPressed(input, inputs[i].down_action)) {
      velocities[i].y = DOWN_DIRECTION * properties->PADDLE_SPEED;
    }
  }
}

int main(void) {
  const Properties properties = {
      .SCREEN_WIDTH = 1280,
      .SCREEN_HEIGHT = 720,
      .FPS_LOCK = 60,
      .PADDLE_SPEED = 350.f,
      .PADDLE_WIDTH = 20.f,
      .PADDLE_HEIGHT = 80.f,
      .PADDLE_SCREEN_SIZE_MARGIN = 50.f,
      .BALL_SIDE = 10.f,
      .BALL_INITIAL_VELOCITY_X = 300.f,
      .BALL_INITIAL_VELOCITY_Y = -500.f,
      .WALL_THICKNESS = 10.f,
  };

  InitWindow(properties.SCREEN_WIDTH, properties.SCREEN_HEIGHT, "game");
  SetTargetFPS(properties.FPS_LOCK);

  ecs_world_t *world = ecs_init();

  ECS_COMPONENT(world, Position);
  ECS_COMPONENT(world, Velocity);
  ECS_COMPONENT(world, RenderableRectangle);
  ECS_COMPONENT(world, PlayerInput);
  ECS_COMPONENT(world, Collider);

  ECS_TAG(world, Ball);
  ECS_TAG(world, Paddle);
  ECS_TAG(world, Wall);

  ECS_SYSTEM(world, Move, EcsOnUpdate, Position, [in] Velocity);
  ECS_SYSTEM(world, RenderRectangle,
             EcsOnUpdate, [in] Position, [in] RenderableRectangle);
  ECS_SYSTEM(world, HandlePlayerInputActions,
             EcsOnUpdate, [out] Velocity, [in] PlayerInput);
  ECS_SYSTEM(world, BallPaddleCollisions, EcsOnUpdate,
             Position, [inout] Velocity, [in] Collider, Ball);

  ECS_SYSTEM(world, BallWallCollisions, EcsOnUpdate,
             Position, [inout] Velocity, [in] Collider, Ball);

  Input left_paddle_input;
  InitInput(&left_paddle_input);
  left_paddle_input.bindings[ACTION_MOVE_UP].key = KEY_W;
  left_paddle_input.bindings[ACTION_MOVE_DOWN].key = KEY_S;

  Input right_paddle_input;
  InitInput(&right_paddle_input);
  right_paddle_input.bindings[ACTION_MOVE_UP].key = KEY_UP;
  right_paddle_input.bindings[ACTION_MOVE_DOWN].key = KEY_DOWN;

  ecs_entity_t left_paddle =
      ecs_insert(world,
                 ecs_value(Position, {properties.PADDLE_SCREEN_SIZE_MARGIN,
                                      (float)properties.SCREEN_HEIGHT / 2 -
                                          properties.PADDLE_HEIGHT / 2}),
                 ecs_value(Velocity, {0, 0}),
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
                 ecs_value(Collider, {
                                         .width = properties.PADDLE_WIDTH,
                                         .height = properties.PADDLE_HEIGHT,
                                     }));

  ecs_add_id(world, left_paddle, Paddle);

  ecs_entity_t right_paddle =
      ecs_insert(world,
                 ecs_value(Position, {properties.SCREEN_WIDTH -
                                          properties.PADDLE_SCREEN_SIZE_MARGIN -
                                          properties.PADDLE_WIDTH,
                                      (float)properties.SCREEN_HEIGHT / 2 -
                                          properties.PADDLE_HEIGHT / 2}),
                 ecs_value(Velocity, {0, 0}),
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
                 ecs_value(Collider, {
                                         .width = properties.PADDLE_WIDTH,
                                         .height = properties.PADDLE_HEIGHT,
                                     }));

  ecs_add_id(world, right_paddle, Paddle);

  ecs_entity_t ball = ecs_insert(
      world,
      ecs_value(Position, {.x = (float)properties.SCREEN_WIDTH / 2,
                           .y = (float)properties.SCREEN_HEIGHT / 2}),
      ecs_value(Velocity, {.x = properties.BALL_INITIAL_VELOCITY_X,
                           .y = properties.BALL_INITIAL_VELOCITY_Y}),
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
          {.x = 0, .y = properties.SCREEN_HEIGHT - properties.WALL_THICKNESS}),
      ecs_value(Collider,
                {properties.SCREEN_WIDTH, properties.WALL_THICKNESS}));

  ecs_add_id(world, upper_wall, Wall);
  ecs_add_id(world, lower_wall, Wall);

  while (!WindowShouldClose()) {
    UpdateInput(&left_paddle_input);
    UpdateInput(&right_paddle_input);

    ecs_run(world, ecs_id(BallPaddleCollisions), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(BallWallCollisions), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(HandlePlayerInputActions), GetFrameTime(),
            (void *)&properties);
    ecs_run(world, ecs_id(Move), GetFrameTime(), (void *)&properties);

    BeginDrawing();
    ClearBackground(BLACK);
    ecs_run(world, ecs_id(RenderRectangle), GetFrameTime(), NULL);
    EndDrawing();
  }

  CloseWindow();
  ecs_fini(world);
  return 0;
}

// TODO: clamp player movement
// TODO: add score
