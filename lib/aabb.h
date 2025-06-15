#ifndef AABB_H
#define AABB_H

/*

Currently I`ll support only rectangle colliders

*/

#include "flecs.h"
#include "movement.h"

typedef struct Collider {
  float width;
  float height;
} Collider;

typedef struct Rect {
  float x;
  float y;
  float width;
  float height;
} Rect;

int Intersect(float x_1_min, float x_1_max, float y_1_min, float y_1_max,
              float x_2_min, float x_2_max, float y_2_min, float y_2_max);
int IntersectRects(Rect r_1, Rect r_2);

void HandleCollision(ecs_entity_t entity_a, ecs_entity_t entity_b,
                     Position *pos_a, Position *pos_b, Collider *col_a,
                     Collider *col_b);

void CheckCollision(ecs_iter_t *it);
void CheckBallPaddleCollision(ecs_iter_t *it);

#endif // !AABB_H
