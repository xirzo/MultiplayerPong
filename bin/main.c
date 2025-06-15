#include "aabb.h"
#include "flecs.h"
#include "input.h"
#include "movement.h"
#include "properties.h"
#include "raylib.h"
#include "render.h"
#include <stddef.h>
#include <stdio.h>

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

  for (size_t i = 0; i < it->count; i++) {
    velocities[i].y = 0;
    Input *input = inputs[i].input;

    if (IsActionPressed(input, inputs[i].up_action)) {
      velocities[i].y = UP_DIRECTION;
    }
    if (IsActionPressed(input, inputs[i].down_action)) {
      velocities[i].y = DOWN_DIRECTION;
    }
  }
}

int main(void) {
  const Properties properties = {
      .SCREEN_WIDTH = 1280,
      .SCREEN_HEIGHT = 720,
      .FPS_LOCK = 60,
      .PADDLE_SPEED = 100.f,
      .PADDLE_WIDTH = 20.f,
      .PADDLE_HEIGHT = 80.f,
      .PADDLE_SCREEN_SIZE_MARGIN = 50.f,
      .BALL_SIDE = 10.f,
  };

  InitWindow(properties.SCREEN_WIDTH, properties.SCREEN_HEIGHT, "game");
  SetTargetFPS(properties.FPS_LOCK);

  ecs_world_t *world = ecs_init();

  ECS_COMPONENT(world, Position);
  ECS_COMPONENT(world, Velocity);
  ECS_COMPONENT(world, RenderableRectangle);
  ECS_COMPONENT(world, PlayerInput);
  ECS_COMPONENT(world, Collider);

  ECS_SYSTEM(world, Move, EcsOnUpdate, Position, [in] Velocity);
  ECS_SYSTEM(world, RenderRectangle,
             EcsOnUpdate, [in] Position, [in] RenderableRectangle);
  ECS_SYSTEM(world, HandlePlayerInputActions,
             EcsOnUpdate, [out] Velocity, [in] PlayerInput);

  ECS_SYSTEM(world, CheckCollision, EcsOnUpdate, [in] Position, [in] Collider);

  Input left_paddle_input;
  InitInput(&left_paddle_input);
  left_paddle_input.bindings[ACTION_MOVE_UP].key = KEY_S;
  left_paddle_input.bindings[ACTION_MOVE_DOWN].key = KEY_D;

  Input right_paddle_input;
  InitInput(&right_paddle_input);

  right_paddle_input.bindings[ACTION_MOVE_UP].key = KEY_J;
  right_paddle_input.bindings[ACTION_MOVE_DOWN].key = KEY_K;

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
                 ecs_value(Collider,
                           {
                               .width = properties.PADDLE_WIDTH,
                               .height = properties.PADDLE_HEIGHT,
                           })

      );

  ecs_entity_t right_paddle =
      ecs_insert(world,
                 ecs_value(Position, {properties.PADDLE_SCREEN_SIZE_MARGIN,
                                      (float)properties.SCREEN_HEIGHT / 2 -
                                          properties.PADDLE_HEIGHT / 2 + 100}),
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
                 ecs_value(Collider,
                           {
                               .width = properties.PADDLE_WIDTH,
                               .height = properties.PADDLE_HEIGHT,
                           })

      );

  ecs_entity_t ball = ecs_insert(
      world,
      ecs_value(Position, {.x = (float)properties.SCREEN_WIDTH / 2,
                           .y = (float)properties.SCREEN_HEIGHT / 2}),
      ecs_value(Velocity, {.x = 0, .y = 0}),
      ecs_value(RenderableRectangle,
                {properties.BALL_SIDE, properties.BALL_SIDE, WHITE}),
      ecs_value(Collider, {properties.BALL_SIDE, properties.BALL_SIDE}));

  while (!WindowShouldClose()) {
    UpdateInput(&left_paddle_input);
    UpdateInput(&right_paddle_input);

    ecs_run(world, ecs_id(CheckCollision), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(HandlePlayerInputActions), GetFrameTime(), NULL);
    ecs_run(world, ecs_id(Move), GetFrameTime(), (void *)&properties);

    BeginDrawing();

    ecs_run(world, ecs_id(RenderRectangle), GetFrameTime(), NULL);
    ClearBackground(BLACK);

    EndDrawing();
  }

  CloseWindow();
  ecs_fini(world);
  return 0;
}
