#include "sound.h"
#include <raylib.h>

#include "aabb.h"

void BallPaddleSoundOnCollission(ecs_iter_t* it) {
  Position* ball_positions = ecs_field(it, Position, 0);
  Collider* ball_colliders = ecs_field(it, Collider, 1);
  GameSound* game_sounds = ecs_field(it, GameSound, 2);

  ecs_world_t* world = it->world;

  ecs_query_t* paddle_query =
      ecs_query(world, {.terms = {
                            {.id = ecs_lookup(world, "Position")},
                            {.id = ecs_lookup(world, "Collider")},
                            {.id = ecs_lookup(world, "Paddle")},
                        }});

  for (int i = 0; i < it->count; i++) {
    Position* ball_pos = &ball_positions[i];
    Collider* ball_col = &ball_colliders[i];
    GameSound* game_sound = &game_sounds[i];

    ecs_iter_t paddle_it = ecs_query_iter(world, paddle_query);

    while (ecs_query_next(&paddle_it)) {
      Position* paddle_positions = ecs_field(&paddle_it, Position, 0);
      Collider* paddle_colliders = ecs_field(&paddle_it, Collider, 1);

      for (int j = 0; j < paddle_it.count; j++) {
        Position* paddle_position = &paddle_positions[j];
        Collider* paddle_collider = &paddle_colliders[j];

        int collision = IntersectRects(
            (Rect){ball_pos->x, ball_pos->y, ball_col->width, ball_col->height},
            (Rect){paddle_position->x, paddle_position->y,
                   paddle_collider->width, paddle_collider->height});

        if (collision) {
          PlaySound(game_sound->sound);
        }
      }
    }
  }

  ecs_query_fini(paddle_query);
}

void BallWallSoundOnCollission(ecs_iter_t* it) {
  Position* ball_positions = ecs_field(it, Position, 0);
  Collider* ball_colliders = ecs_field(it, Collider, 1);
  GameSound* game_sounds = ecs_field(it, GameSound, 2);

  ecs_world_t* world = it->world;

  ecs_query_t* wall_query =
      ecs_query(world, {.terms = {{.id = ecs_lookup(world, "Position")},
                                  {.id = ecs_lookup(world, "Collider")},
                                  {.id = ecs_lookup(world, "Wall")}}});

  for (int i = 0; i < it->count; i++) {
    Position* ball_pos = &ball_positions[i];
    Collider* ball_col = &ball_colliders[i];
    GameSound* game_sound = &game_sounds[i];

    ecs_iter_t wall_it = ecs_query_iter(world, wall_query);

    while (ecs_query_next(&wall_it)) {
      Position* wall_positions = ecs_field(&wall_it, Position, 0);
      Collider* wall_colliders = ecs_field(&wall_it, Collider, 1);

      for (int j = 0; j < wall_it.count; j++) {
        Position* wall_pos = &wall_positions[j];
        Collider* wall_col = &wall_colliders[j];

        int collision = IntersectRects(
            (Rect){ball_pos->x, ball_pos->y, ball_col->width, ball_col->height},
            (Rect){wall_pos->x, wall_pos->y, wall_col->width,
                   wall_col->height});

        if (collision) {
          PlaySound(game_sound->sound);
        }
      }
    }
  }

  ecs_query_fini(wall_query);
}
