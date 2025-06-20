#ifndef AABB_H
#define AABB_H

#include "flecs.h"
#include "movement.h"

typedef struct Rect {
    float x, y, width, height;
} Rect;

typedef struct Collider {
    float width;
    float height;
} Collider;

int Intersect(
    float x_1_min,
    float x_1_max,
    float y_1_min,
    float y_1_max,
    float x_2_min,
    float x_2_max,
    float y_2_min,
    float y_2_max
);

int IntersectRects(Rect r_1, Rect r_2);

void BounceBallWithMovement(
    Position     *ball_pos,
    BallMovement *ball_movement,
    float         ball_width,
    float         ball_height,
    float         target_x,
    float         target_y,
    float         target_width,
    float         target_height
);

void BallWallCollisions(ecs_iter_t *it);
void BallPaddleCollisions(ecs_iter_t *it);

#endif  // !AABB_H
