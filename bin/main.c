#include <stddef.h>
#include "raylib.h"
#include "flecs.h"

typedef struct Properties {
  int SCREEN_WIDTH;
  int SCREEN_HEIGHT;
  int FPS_LOCK;
  float PADDLE_SPEED;
} Properties;

typedef struct Position {
  float x;
  float y;
} Position, Velocity;

typedef struct RenderableRectangle {
  float width;
  float height;
  Color color;
} RenderableRectangle;

void Move(ecs_iter_t* it) {
  Position* positions = ecs_field(it, Position, 0);
  Velocity* velocities = ecs_field(it, Velocity, 1);

  const Properties* properties = (Properties*)it->param;

  for (size_t i = 0; i < it->count; i++) {
    positions[i].x += velocities[i].x * it->delta_time * properties->PADDLE_SPEED;
    positions[i].y += velocities[i].y * it->delta_time * properties->PADDLE_SPEED;
  }
}

void RenderRectangle(ecs_iter_t* it) {
  Position* positions = ecs_field(it, Position, 0);
  RenderableRectangle* rectangles = ecs_field(it, RenderableRectangle, 1);

  for (size_t i = 0; i < it->count; i++) {
    Position* p = &positions[i];
    RenderableRectangle* r = &rectangles[i];

    DrawRectangle(p->x, p->y, r->width, r->height, r->color);
  }
}

int main(void) {
  const Properties properties = {
      .SCREEN_WIDTH = 1280,
      .SCREEN_HEIGHT = 720,
      .FPS_LOCK = 60,
      .PADDLE_SPEED = 100.f,
  };

  InitWindow(properties.SCREEN_WIDTH, properties.SCREEN_HEIGHT, "game");
  SetTargetFPS(properties.FPS_LOCK);

  ecs_world_t* world = ecs_init();

  ECS_COMPONENT(world, Position);
  ECS_COMPONENT(world, Velocity);
  ECS_COMPONENT(world, RenderableRectangle);

  ECS_SYSTEM(world, Move, EcsOnUpdate, Position, [in] Velocity);
  ECS_SYSTEM(world, RenderRectangle, EcsOnUpdate, [in] Position, [in] RenderableRectangle);

  ecs_entity_t left_paddle =
      ecs_insert(world, ecs_value(Position, {10, 20}), ecs_value(Velocity, {1, 2}),
                 ecs_value(RenderableRectangle, {
                                                    .width = 80.f,
                                                    .height = 20.f,
                                                    .color = RED,
                                                }));

  while (!WindowShouldClose()) {
    ecs_run(world, ecs_id(Move), GetFrameTime(), (void*)&properties);

    BeginDrawing();

    ecs_run(world, ecs_id(RenderRectangle), GetFrameTime(), NULL);
    ClearBackground(BLACK);

    EndDrawing();
  }

  CloseWindow();
  ecs_fini(world);
  return 0;
}
