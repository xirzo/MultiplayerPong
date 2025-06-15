#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "flecs.h"
#include "utils.h"

#define BOUNCE_SPEED_INCREASE_MULTIPLIER 1.02f

typedef struct Position {
  float x;
  float y;
} Position;

typedef struct BallMovement {
  float min_speed;
  float max_speed;
  vec2 direction;
  vec2 velocity;
} BallMovement;

typedef struct PaddleMovement {
  float speed;
  vec2 direction;
  vec2 velocity;
} PaddleMovement;

typedef struct MovementClamp {
  float lower_limit;
  float upper_limit;
} MovementClamp;

void MoveBall(ecs_iter_t *it);
void MovePaddle(ecs_iter_t *it);
void ClampPosition(ecs_iter_t *it);

#endif // !MOVEMENT_H
