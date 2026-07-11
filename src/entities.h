#ifndef ENTITIES_H_
#define ENTITIES_H_

#include "flecs.h"
#include "input.h"
#include "raylib.h"

typedef enum {
  kUpperWall = 0,
  kLowerWall = 1,
} WallPosition;

ecs_entity_t CreatePlayerEntity(ecs_world_t* world, Input* input);
ecs_entity_t CreateEnemyEntity(ecs_world_t* world);
ecs_entity_t CreateBallEntity(ecs_world_t* world, Sound collision_sound);
ecs_entity_t CreateWallEntity(ecs_world_t* world, float x, float y,
                               WallPosition position);

#endif  // ENTITIES_H_
