#include "entities.h"

#include "aabb.h"
#include "components.h"
#include "input.h"
#include "movement.h"
#include "player.h"
#include "properties.h"
#include "render.h"
#include "score.h"
#include "sound.h"

ecs_entity_t CreatePlayerEntity(ecs_world_t* world, Input* input) {
  ecs_entity_t player = ecs_insert(
      world,
      ecs_value(Position, {g_Properties.PADDLE_SCREEN_SIZE_MARGIN,
                           (float)g_Properties.SCREEN_HEIGHT / 2 -
                               g_Properties.PADDLE_HEIGHT / 2}),
      ecs_value(PaddleMovement,
                {
                    .speed = g_Properties.PADDLE_SPEED,
                    .direction = {0, 0},
                    .velocity = {0, 0},
                }),
      ecs_value(RenderableRectangle,
                {
                    .width = g_Properties.PADDLE_WIDTH,
                    .height = g_Properties.PADDLE_HEIGHT,
                    .color = RED,
                }),
      ecs_value(PlayerInput,
                {
                    .up_action = ACTION_MOVE_UP,
                    .down_action = ACTION_MOVE_DOWN,
                    .input = input,
                }),
      ecs_value(Player,
                {
                    .is_main = 0,
                }),
      ecs_value(Collider,
                {
                    .width = g_Properties.PADDLE_WIDTH,
                    .height = g_Properties.PADDLE_HEIGHT,
                }),
      ecs_value(MovementClamp,
                {
                    .lower_limit = 0,
                    .upper_limit =
                        g_Properties.SCREEN_HEIGHT - g_Properties.PADDLE_HEIGHT,
                }),
      ecs_value(Score, {.value = 0}));

  ecs_add_id(world, player, Paddle);
  return player;
}

ecs_entity_t CreateEnemyEntity(ecs_world_t* world) {
  ecs_entity_t enemy = ecs_insert(
      world,
      ecs_value(Position, {g_Properties.SCREEN_WIDTH -
                               g_Properties.PADDLE_SCREEN_SIZE_MARGIN -
                               g_Properties.PADDLE_WIDTH,
                           (float)g_Properties.SCREEN_HEIGHT / 2 -
                               g_Properties.PADDLE_HEIGHT / 2}),
      ecs_value(RenderableRectangle,
                {
                    .width = g_Properties.PADDLE_WIDTH,
                    .height = g_Properties.PADDLE_HEIGHT,
                    .color = GREEN,
                }),
      ecs_value(Collider,
                {
                    .width = g_Properties.PADDLE_WIDTH,
                    .height = g_Properties.PADDLE_HEIGHT,
                }),
      ecs_value(Score, {.value = 0}));

  ecs_add_id(world, enemy, Enemy);
  ecs_add_id(world, enemy, Paddle);
  return enemy;
}

ecs_entity_t CreateBallEntity(ecs_world_t* world, Sound collision_sound) {
  ecs_entity_t ball = ecs_insert(
      world,
      ecs_value(Position, {.x = (float)g_Properties.SCREEN_WIDTH / 2,
                           .y = (float)g_Properties.SCREEN_HEIGHT / 2}),
      ecs_value(BallMovement,
                {
                    .current_speed = g_Properties.BALL_MIN_SPEED,
                    .min_speed = g_Properties.BALL_MIN_SPEED,
                    .max_speed = g_Properties.BALL_MAX_SPEED,
                    .direction = g_Properties.BALL_INITIAL_DIRECTION,
                    .velocity = {0, 0},
                }),
      ecs_value(GameSound,
                {
                    .sound = collision_sound,
                }),
      ecs_value(RenderableRectangle,
                {g_Properties.BALL_SIDE, g_Properties.BALL_SIDE, WHITE}),
      ecs_value(Collider, {g_Properties.BALL_SIDE, g_Properties.BALL_SIDE}));

  ecs_add_id(world, ball, Ball);
  return ball;
}

ecs_entity_t CreateWallEntity(ecs_world_t* world, float x, float y,
                               WallPosition position) {
  (void)position;
  ecs_entity_t wall = ecs_insert(
      world,
      ecs_value(Position, {.x = x, .y = y}),
      ecs_value(Collider,
                {g_Properties.SCREEN_WIDTH, g_Properties.WALL_THICKNESS}));

  ecs_add_id(world, wall, Wall);
  return wall;
}
