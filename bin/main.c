#include <stddef.h>
#include "raylib.h"
#include "flecs.h"

typedef struct Properties {
  int SCREEN_WIDTH;
  int SCREEN_HEIGHT;
} Properties;

int main(void) {
  const Properties properties = {
      .SCREEN_WIDTH = 800,
      .SCREEN_HEIGHT = 450,
  };

  ecs_world_t *world = ecs_init();

  InitWindow(properties.SCREEN_WIDTH, properties.SCREEN_HEIGHT, "game");

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    EndDrawing();
  }

  CloseWindow();
  ecs_fini(world);
  return 0;
}
