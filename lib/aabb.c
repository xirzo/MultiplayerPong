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

void BounceBall(float *ball_x, float *ball_y, float *ball_vel_x,
                float *ball_vel_y, float ball_width, float ball_height,
                float target_x, float target_y, float target_width,
                float target_height) {
  float ball_center_x = *ball_x + ball_width / 2;
  float ball_center_y = *ball_y + ball_height / 2;
  float paddle_center_x = target_x + target_width / 2;
  float paddle_center_y = target_y + target_height / 2;

  float dx = ball_center_x - paddle_center_x;
  float dy = ball_center_y - paddle_center_y;

  float overlap_x = (ball_width + target_width) / 2 - fabsf(dx);
  float overlap_y = (ball_height + target_height) / 2 - fabsf(dy);

  if (overlap_x < overlap_y) {
    *ball_vel_x = -(*ball_vel_x);

    if (dx > 0) {
      *ball_x = target_x + target_width;
    } else {
      *ball_x = target_x - ball_width;
    }

    float hit_factor = dy / (target_height / 2);
    *ball_vel_y += hit_factor * PADDLE_SPIN_STRENGTH;

  } else {
    *ball_vel_y = -(*ball_vel_y);

    if (dy > 0) {
      *ball_y = target_y + target_height;
    } else {
      *ball_y = target_y - ball_height;
    }
  }

  if (fabsf(*ball_vel_x) > PADDLE_MAX_SPEED) {
    *ball_vel_x = (*ball_vel_x > 0) ? PADDLE_MAX_SPEED : -PADDLE_MAX_SPEED;
  }
  if (fabsf(*ball_vel_y) > PADDLE_MAX_SPEED) {
    *ball_vel_y = (*ball_vel_y > 0) ? PADDLE_MAX_SPEED : -PADDLE_MAX_SPEED;
  }
}

void BallWallCollisions(ecs_iter_t *it) {
  Position *ball_positions = ecs_field(it, Position, 0);
  Velocity *ball_velocities = ecs_field(it, Velocity, 1);
  Collider *ball_colliders = ecs_field(it, Collider, 2);

  ecs_world_t *world = it->world;

  ecs_query_t *wall_query =
      ecs_query(world, {.terms = {{.id = ecs_lookup(world, "Position")},
                                  {.id = ecs_lookup(world, "Collider")},
                                  {.id = ecs_lookup(world, "Wall")}}});

  for (size_t i = 0; i < it->count; i++) {
    Position *ball_pos = &ball_positions[i];
    Velocity *ball_vel = &ball_velocities[i];
    Collider *ball_col = &ball_colliders[i];

    ecs_iter_t wall_it = ecs_query_iter(it->world, wall_query);

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
          BounceBall(&ball_pos->x, &ball_pos->y, &ball_vel->x, &ball_vel->y,
                     ball_col->width, ball_col->height, wall_pos->x,
                     wall_pos->y, wall_col->width, wall_col->height);
        }
      }
    }
  }
}

void BallPaddleCollisions(ecs_iter_t *it) {
  Position *ball_positions = ecs_field(it, Position, 0);
  Velocity *ball_velocities = ecs_field(it, Velocity, 1);
  Collider *ball_colliders = ecs_field(it, Collider, 2);

  PaddleData *paddle_data = (PaddleData *)it->param;
  ecs_world_t *world = it->world;

  ecs_entity_t position_id = ecs_lookup(world, "Position");
  ecs_entity_t collider_id = ecs_lookup(world, "Collider");

  for (size_t i = 0; i < it->count; i++) {
    Position *ball_pos = &ball_positions[i];
    Velocity *ball_vel = &ball_velocities[i];
    Collider *ball_col = &ball_colliders[i];

    const Position *left_pos =
        ecs_get_id(world, paddle_data->left_paddle, position_id);
    const Collider *left_col =
        ecs_get_id(world, paddle_data->left_paddle, collider_id);

    if (left_pos && left_col) {
      int collision = IntersectRects(
          (Rect){ball_pos->x, ball_pos->y, ball_col->width, ball_col->height},
          (Rect){left_pos->x, left_pos->y, left_col->width, left_col->height});

      if (collision) {
        BounceBall(&ball_pos->x, &ball_pos->y, &ball_vel->x, &ball_vel->y,
                   ball_col->width, ball_col->height, left_pos->x, left_pos->y,
                   left_col->width, left_col->height);
      }
    }

    const Position *right_pos =
        ecs_get_id(world, paddle_data->right_paddle, position_id);
    const Collider *right_col =
        ecs_get_id(world, paddle_data->right_paddle, collider_id);

    if (right_pos && right_col) {
      int collision = IntersectRects(
          (Rect){ball_pos->x, ball_pos->y, ball_col->width, ball_col->height},
          (Rect){right_pos->x, right_pos->y, right_col->width,
                 right_col->height});

      if (collision) {
        BounceBall(&ball_pos->x, &ball_pos->y, &ball_vel->x, &ball_vel->y,
                   ball_col->width, ball_col->height, right_pos->x,
                   right_pos->y, right_col->width, right_col->height);
      }
    }
  }
}
