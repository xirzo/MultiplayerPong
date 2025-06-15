#ifndef AABB_H
#define AABB_H

/*

Currently I`ll support only rectangle colliders

*/

#include "flecs.h"

#define PADDLE_SPIN_STRENGTH 20.0f
#define PADDLE_MAX_SPEED 4.0f

typedef struct PaddleData {
  ecs_entity_t left_paddle;
  ecs_entity_t right_paddle;
} PaddleData;

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

void BallPaddleCollisions(ecs_iter_t *it);
#endif // !AABB_H
