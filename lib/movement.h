#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "flecs.h"
#include "utils.h"

#define BOUNCE_SPEED_INCREASE_MULTIPLIER 1.01f

typedef struct Position {
  float x;
  float y;
} Position;

typedef struct BallMovement {
  float min_speed;
  float max_speed;
  float current_speed;
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
void MovePlayerSystem(ecs_iter_t *it);
void ClampMovementSystem(ecs_iter_t *it);

#endif // !MOVEMENT_H
