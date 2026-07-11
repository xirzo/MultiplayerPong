#ifndef SYSTEMS_H_
#define SYSTEMS_H_

#include "flecs.h"

void RegisterAllComponents(ecs_world_t* world);

void RegisterAllSystems(ecs_world_t* world);

extern ecs_entity_t g_ScoreCountSystem;
extern ecs_entity_t g_MoveBall;
extern ecs_entity_t g_MovePlayerSystem;
extern ecs_entity_t g_ClampMovementSystem;
extern ecs_entity_t g_RenderRectangle;
extern ecs_entity_t g_ProcessInputSystem;
extern ecs_entity_t g_BallPaddleCollisions;
extern ecs_entity_t g_BallWallCollisions;
extern ecs_entity_t g_BallPaddleSoundOnCollission;
extern ecs_entity_t g_BallWallSoundOnCollission;
extern ecs_entity_t g_MoveEnemySystem;
extern ecs_entity_t g_UpdateBallFromNetwork;
extern ecs_entity_t g_SendPlayerPositionSystem;

#endif  // SYSTEMS_H_
