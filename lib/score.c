#include "score.h"
#include "movement.h"
#include "properties.h"

#include "movement.h"
#include "properties.h"
#include "score.h"

void BallScoringSystem(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  BallMovement *movements = ecs_field(it, BallMovement, 1);

  Properties *props = (Properties *)it->param;

  for (size_t i = 0; i < it->count; i++) {
    Position *ball_pos = &positions[i];
    BallMovement *ball_movement = &movements[i];

    if (ball_pos->x < 0) {
      ecs_query_desc_t desc = {0};
      desc.terms[0].id = ecs_lookup(it->world, "Score");
      desc.terms[1].id = ecs_lookup(it->world, "Paddle");

      ecs_query_t *paddle_query = ecs_query_init(it->world, &desc);

      if (paddle_query) {
        ecs_iter_t paddle_it = ecs_query_iter(it->world, paddle_query);
        int paddle_count = 0;

        while (ecs_query_next(&paddle_it)) {
          Score *scores = ecs_field(&paddle_it, Score, 0);

          for (int j = 0; j < paddle_it.count; j++) {
            if (paddle_count == 1) {
              scores[j].value++;
              break;
            }
            paddle_count++;
          }
          if (paddle_count > 1)
            break;
        }

        ecs_query_fini(paddle_query);
      }

      ball_pos->x = (float)props->SCREEN_WIDTH / 2;
      ball_pos->y = (float)props->SCREEN_HEIGHT / 2;
      ball_movement->direction = props->BALL_INITIAL_DIRECTION;
      ball_movement->current_speed = props->BALL_MIN_SPEED;

    } else if (ball_pos->x > props->SCREEN_WIDTH) {
      ecs_query_desc_t desc = {0};
      desc.terms[0].id = ecs_lookup(it->world, "Score");
      desc.terms[1].id = ecs_lookup(it->world, "Paddle");

      ecs_query_t *paddle_query = ecs_query_init(it->world, &desc);

      if (paddle_query) {
        ecs_iter_t paddle_it = ecs_query_iter(it->world, paddle_query);
        int paddle_count = 0;

        while (ecs_query_next(&paddle_it)) {
          Score *scores = ecs_field(&paddle_it, Score, 0);

          for (int j = 0; j < paddle_it.count; j++) {
            if (paddle_count == 0) {
              scores[j].value++;
              break;
            }
            paddle_count++;
          }
          if (paddle_count > 0)
            break;
        }

        ecs_query_fini(paddle_query);
      }

      ball_pos->x = (float)props->SCREEN_WIDTH / 2;
      ball_pos->y = (float)props->SCREEN_HEIGHT / 2;
      ball_movement->direction.x = -props->BALL_INITIAL_DIRECTION.x;
      ball_movement->direction.y = props->BALL_INITIAL_DIRECTION.y;
      ball_movement->current_speed = props->BALL_MIN_SPEED;
    }
  }
}
