#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "flecs.h"

typedef struct Position {
  float x;
  float y;
} Position;

typedef struct Velocity {
  float x;
  float y;
} Velocity;

void Move(ecs_iter_t *it);

#endif // !MOVEMENT_H
