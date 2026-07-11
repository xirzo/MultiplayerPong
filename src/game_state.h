#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include "flecs.h"
#include "raylib.h"
#include "server.h"

typedef struct {
  ecs_world_t* world;
  UDPClient* client;
  Sound collision_sound;
  ecs_entity_t player_entity;
  ecs_entity_t enemy_entity;
  ecs_entity_t ball_entity;
  ecs_entity_t upper_wall_entity;
  ecs_entity_t lower_wall_entity;
} GameState;

#endif  // GAME_STATE_H_
