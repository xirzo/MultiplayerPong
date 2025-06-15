#include "aabb.h"
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

void HandleCollision(ecs_entity_t entity_a, ecs_entity_t entity_b,
                     Position *pos_a, Position *pos_b, Collider *col_a,
                     Collider *col_b) {
  float overlap_x = (pos_a->x + col_a->width) - pos_b->x;

  if (pos_a->x > pos_b->x) {
    overlap_x = (pos_b->x + col_b->width) - pos_a->x;
  }

  float overlap_y = (pos_a->y + col_a->height) - pos_b->y;
  if (pos_a->y > pos_b->y) {
    overlap_y = (pos_b->y + col_b->height) - pos_a->y;
  }

  if (overlap_x < overlap_y) {
    if (pos_a->x < pos_b->x) {
      pos_a->x -= overlap_x / 2;
      pos_b->x += overlap_x / 2;
    } else {
      pos_a->x += overlap_x / 2;
      pos_b->x -= overlap_x / 2;
    }
  } else {
    if (pos_a->y < pos_b->y) {
      pos_a->y -= overlap_y / 2;
      pos_b->y += overlap_y / 2;
    } else {
      pos_a->y += overlap_y / 2;
      pos_b->y -= overlap_y / 2;
    }
  }
}

void CheckCollision(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  Collider *colliders = ecs_field(it, Collider, 1);

  for (size_t i = 0; i < it->count; i++) {
    for (size_t j = i + 1; j < it->count; j++) {
      Position *pos_a = &positions[i];
      Position *pos_b = &positions[j];
      Collider *col_a = &colliders[i];
      Collider *col_b = &colliders[j];

      int res = IntersectRects((Rect){.x = pos_a->x,
                                      .y = pos_a->y,
                                      .width = col_a->width,
                                      .height = col_a->height},
                               (Rect){

                                   .x = pos_b->x,
                                   .y = pos_b->y,
                                   .width = col_b->width,

                                   .height = col_b->height});

      if (res) {
        HandleCollision(it->entities[i], it->entities[j], pos_a, pos_b, col_a,
                        col_b);
      }
    }
  }
}

void CheckBallPaddleCollision(ecs_iter_t *it) {

  Position *positions = ecs_field(it, Position, 0);
  Collider *colliders = ecs_field(it, Collider, 1);

  for (size_t i = 0; i < it->count; i++) {
    for (size_t j = i + 1; j < it->count; j++) {
      Position *pos_a = &positions[i];
      Position *pos_b = &positions[j];
      Collider *col_a = &colliders[i];
      Collider *col_b = &colliders[j];

      int res = IntersectRects((Rect){.x = pos_a->x,
                                      .y = pos_a->y,
                                      .width = col_a->width,
                                      .height = col_a->height},
                               (Rect){

                                   .x = pos_b->x,
                                   .y = pos_b->y,
                                   .width = col_b->width,

                                   .height = col_b->height});

      if (res) {
        HandleCollision(it->entities[i], it->entities[j], pos_a, pos_b, col_a,
                        col_b);
      }
    }
  }
}
