#include "render.h"
#include "movement.h"

void RenderRectangle(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  RenderableRectangle *rectangles = ecs_field(it, RenderableRectangle, 1);

  for (size_t i = 0; i < it->count; i++) {
    Position *p = &positions[i];
    RenderableRectangle *r = &rectangles[i];

    DrawRectangle(p->x, p->y, r->width, r->height, r->color);
  }
}
