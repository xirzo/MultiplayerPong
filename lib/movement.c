#include "movement.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>

void MoveBall(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  BallMovement *movements = ecs_field(it, BallMovement, 1);

  for (size_t i = 0; i < it->count; i++) {
    Position *pos = &positions[i];
    BallMovement *movement = &movements[i];

    if (movement->current_speed < movement->min_speed) {
      movement->current_speed = movement->min_speed;
    } else if (movement->current_speed > movement->max_speed) {
      movement->current_speed = movement->max_speed;
    }

    float direction_magnitude = vec2_magnitude(&movement->direction);

    vec2_normalize(&movement->direction);

    vec2 displacement = movement->direction;
    vec2_multiply(&displacement, movement->current_speed * it->delta_time);

    pos->x += displacement.x;
    pos->y += displacement.y;

    movement->velocity = movement->direction;
    vec2_multiply(&movement->velocity, movement->current_speed);
  }
}

void MovePaddle(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  PaddleMovement *movements = ecs_field(it, PaddleMovement, 1);

  for (size_t i = 0; i < it->count; i++) {
    Position *pos = &positions[i];
    PaddleMovement *movement = &movements[i];

    pos->x += movement->direction.x * movement->speed * it->delta_time;
    pos->y += movement->direction.y * movement->speed * it->delta_time;
  }
}

void ClampPosition(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  MovementClamp *clamps = ecs_field(it, MovementClamp, 1);

  for (size_t i = 0; i < it->count; i++) {
    Position *pos = &positions[i];
    MovementClamp *clamp = &clamps[i];

    if (pos->y < clamp->lower_limit) {
      pos->y = clamp->lower_limit;
    }

    if (pos->y > clamp->upper_limit) {
      pos->y = clamp->upper_limit;
    }
  }
}
