#include "movement.h"
#include "properties.h"

void Move(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  Velocity *velocities = ecs_field(it, Velocity, 1);

  const Properties *properties = (Properties *)it->param;

  for (size_t i = 0; i < it->count; i++) {
    positions[i].x +=
        velocities[i].x * it->delta_time * properties->PADDLE_SPEED;
    positions[i].y +=
        velocities[i].y * it->delta_time * properties->PADDLE_SPEED;
  }
}
