#include "movement.h"
#include "utils.h"

void MoveBall(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  BallMovement *movements = ecs_field(it, BallMovement, 1);

  for (size_t i = 0; i < it->count; i++) {
    Position *pos = &positions[i];
    BallMovement *movement = &movements[i];

    float current_speed = vec2_magnitude(&movement->velocity);

    if (current_speed < 0.001f) {
      float direction_magnitude = vec2_magnitude(&movement->direction);
      if (direction_magnitude > 0.001f) {
        current_speed = movement->min_speed;
      }
    }

    if (current_speed < movement->min_speed) {
      current_speed = movement->min_speed;
    } else if (current_speed > movement->max_speed) {
      current_speed = movement->max_speed;
    }

    vec2 normalized_direction = movement->direction;
    vec2_normalize(&normalized_direction);

    vec2 displacement = normalized_direction;
    vec2_multiply(&displacement, current_speed * it->delta_time);

    pos->x += displacement.x;
    pos->y += displacement.y;

    movement->velocity = normalized_direction;
    vec2_multiply(&movement->velocity, current_speed);
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
