#ifndef RENDER_H
#define RENDER_H

#include <raylib.h>
#include "flecs.h"

typedef struct RenderableRectangle {
  float width;
  float height;
  Color color;
} RenderableRectangle;

void RenderRectangle(ecs_iter_t* it);

#endif  // !RENDER_H
