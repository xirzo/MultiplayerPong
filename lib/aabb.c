#include "aabb.h"
#include "movement.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>

int Intersect(float x_1_min, float x_1_max, float y_1_min, float y_1_max,
              float x_2_min, float x_2_max, float y_2_min, float y_2_max) {
  if (x_1_min <= x_2_max && x_1_max >= x_2_min && y_1_min <= y_2_max &&
      y_1_max >= y_2_min) {
    return 1;
  }
  return 0;
}

int IntersectRects(Rect r_1, Rect r_2) {
  return Intersect(r_1.x, r_1.x + r_1.width, r_1.y, r_1.y + r_1.height, r_2.x,
                   r_2.x + r_2.width, r_2.y, r_2.y + r_2.height);
}

void BounceBallWithMovement(Position *ball_pos, BallMovement *ball_movement,
                            float ball_width, float ball_height, float target_x,
                            float target_y, float target_width,
                            float target_height, float spin_strength) {

  float ball_center_x = ball_pos->x + ball_width / 2;
  float ball_center_y = ball_pos->y + ball_height / 2;
  float target_center_x = target_x + target_width / 2;
  float target_center_y = target_y + target_height / 2;

  float dx = ball_center_x - target_center_x;
  float dy = ball_center_y - target_center_y;

  float overlap_x = (ball_width + target_width) / 2 - fabsf(dx);
  float overlap_y = (ball_height + target_height) / 2 - fabsf(dy);

  if (overlap_x < overlap_y) {
    ball_movement->direction.x = -ball_movement->direction.x;

    if (dx > 0) {
      ball_pos->x = target_x + target_width;
    } else {
      ball_pos->x = target_x - ball_width;
    }

    float hit_factor = dy / (target_height / 2);

    ball_movement->direction.y += hit_factor * spin_strength * 0.1f;

  } else {
    ball_movement->direction.y = -ball_movement->direction.y;

    if (dy > 0) {
      ball_pos->y = target_y + target_height;
    } else {
      ball_pos->y = target_y - ball_height;
    }
  }

  vec2_normalize(&ball_movement->direction);

  float current_speed = vec2_magnitude(&ball_movement->velocity);
  if (current_speed < ball_movement->min_speed) {
    current_speed = ball_movement->min_speed;
  }

  current_speed *= BOUNCE_SPEED_INCREASE_MULTIPLIER;

  if (current_speed > ball_movement->max_speed) {
    current_speed = ball_movement->max_speed;
  }

  ball_movement->velocity = ball_movement->direction;
  vec2_multiply(&ball_movement->velocity, current_speed);
}

void BallWallCollisions(ecs_iter_t *it) {
  Position *ball_positions = ecs_field(it, Position, 0);
  BallMovement *ball_movements = ecs_field(it, BallMovement, 1);
  Collider *ball_colliders = ecs_field(it, Collider, 2);

  ecs_world_t *world = it->world;

  ecs_query_t *wall_query =
      ecs_query(world, {.terms = {{.id = ecs_lookup(world, "Position")},
                                  {.id = ecs_lookup(world, "Collider")},
                                  {.id = ecs_lookup(world, "Wall")}}});

  for (size_t i = 0; i < it->count; i++) {
    Position *ball_pos = &ball_positions[i];
    BallMovement *ball_movement = &ball_movements[i];
    Collider *ball_col = &ball_colliders[i];

    ecs_iter_t wall_it = ecs_query_iter(world, wall_query);

    while (ecs_query_next(&wall_it)) {
      Position *wall_positions = ecs_field(&wall_it, Position, 0);
      Collider *wall_colliders = ecs_field(&wall_it, Collider, 1);

      for (size_t j = 0; j < wall_it.count; j++) {
        Position *wall_pos = &wall_positions[j];
        Collider *wall_col = &wall_colliders[j];

        int collision = IntersectRects(
            (Rect){ball_pos->x, ball_pos->y, ball_col->width, ball_col->height},
            (Rect){wall_pos->x, wall_pos->y, wall_col->width,
                   wall_col->height});

        if (collision) {
          BounceBallWithMovement(ball_pos, ball_movement, ball_col->width,
                                 ball_col->height, wall_pos->x, wall_pos->y,
                                 wall_col->width, wall_col->height, 0.0f);
        }
      }
    }
  }

  ecs_query_fini(wall_query);
}

void BallPaddleCollisions(ecs_iter_t *it) {
  Position *ball_positions = ecs_field(it, Position, 0);
  BallMovement *ball_movements = ecs_field(it, BallMovement, 1);
  Collider *ball_colliders = ecs_field(it, Collider, 2);

  ecs_world_t *world = it->world;

  ecs_query_t *paddle_query =
      ecs_query(world, {.terms = {
                            {.id = ecs_lookup(world, "Position")},
                            {.id = ecs_lookup(world, "Collider")},
                            {.id = ecs_lookup(world, "Paddle")},
                        }});

  for (size_t i = 0; i < it->count; i++) {
    Position *ball_pos = &ball_positions[i];
    BallMovement *ball_movement = &ball_movements[i];
    Collider *ball_col = &ball_colliders[i];

    ecs_iter_t paddle_it = ecs_query_iter(world, paddle_query);

    while (ecs_query_next(&paddle_it)) {
      Position *paddle_positions = ecs_field(&paddle_it, Position, 0);
      Collider *paddle_colliders = ecs_field(&paddle_it, Collider, 1);

      for (size_t j = 0; j < paddle_it.count; j++) {
        Position *paddle_position = &paddle_positions[j];
        Collider *paddle_collider = &paddle_colliders[j];

        int collision = IntersectRects(
            (Rect){ball_pos->x, ball_pos->y, ball_col->width, ball_col->height},
            (Rect){paddle_position->x, paddle_position->y,
                   paddle_collider->width, paddle_collider->height});

        if (collision) {
          BounceBallWithMovement(ball_pos, ball_movement, ball_col->width,
                                 ball_col->height, paddle_position->x,
                                 paddle_position->y, paddle_collider->width,
                                 paddle_collider->height, 0.f);
        }
      }
    }
  }

  ecs_query_fini(paddle_query);
}
