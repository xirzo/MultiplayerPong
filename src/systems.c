#include "systems.h"

#include "aabb.h"
#include "components.h"
#include "input.h"
#include "movement.h"
#include "network_systems.h"
#include "player.h"
#include "properties.h"
#include "render.h"
#include "score.h"
#include "server.h"
#include "sound.h"

ecs_entity_t g_ScoreCountSystem;
ecs_entity_t g_MoveBall;
ecs_entity_t g_MovePlayerSystem;
ecs_entity_t g_ClampMovementSystem;
ecs_entity_t g_RenderRectangle;
ecs_entity_t g_ProcessInputSystem;
ecs_entity_t g_BallPaddleCollisions;
ecs_entity_t g_BallWallCollisions;
ecs_entity_t g_BallPaddleSoundOnCollission;
ecs_entity_t g_BallWallSoundOnCollission;
ecs_entity_t g_MoveEnemySystem;
ecs_entity_t g_UpdateBallFromNetwork;
ecs_entity_t g_SendPlayerPositionSystem;

void RegisterAllComponents(ecs_world_t* world) {
  ECS_COMPONENT_DEFINE(world, Position);
  ECS_COMPONENT_DEFINE(world, BallMovement);
  ECS_COMPONENT_DEFINE(world, PaddleMovement);
  ECS_COMPONENT_DEFINE(world, MovementClamp);
  ECS_COMPONENT_DEFINE(world, RenderableRectangle);
  ECS_COMPONENT_DEFINE(world, PlayerInput);
  ECS_COMPONENT_DEFINE(world, Collider);
  ECS_COMPONENT_DEFINE(world, Score);
  ECS_COMPONENT_DEFINE(world, GameSound);
  ECS_COMPONENT_DEFINE(world, Player);

  ECS_TAG_DEFINE(world, Ball);
  ECS_TAG_DEFINE(world, Paddle);
  ECS_TAG_DEFINE(world, Enemy);
  ECS_TAG_DEFINE(world, Wall);
}

void RegisterAllSystems(ecs_world_t* world) {
  ECS_SYSTEM(
      world, ScoreCountSystem, EcsOnUpdate, [out] Position,
      [inout] BallMovement, Ball);
  g_ScoreCountSystem = ecs_id(ScoreCountSystem);
  
  ECS_SYSTEM(
      world, MoveBall, EcsOnUpdate, [out] Position,
      [inout] BallMovement, Ball);
  g_MoveBall = ecs_id(MoveBall);
  
  ECS_SYSTEM(
      world, MovePlayerSystem, EcsOnUpdate, [out] Position,
      [inout] PaddleMovement, Paddle);
  g_MovePlayerSystem = ecs_id(MovePlayerSystem);
  
  ECS_SYSTEM(
      world, ClampMovementSystem, EcsOnUpdate, [out] Position,
      MovementClamp);
  g_ClampMovementSystem = ecs_id(ClampMovementSystem);
  
  ECS_SYSTEM(
      world, RenderRectangle, EcsOnUpdate, [in] Position,
      [in] RenderableRectangle);
  g_RenderRectangle = ecs_id(RenderRectangle);
  
  ECS_SYSTEM(
      world, ProcessInputSystem, EcsOnUpdate, [out] PaddleMovement,
      [in] PlayerInput);
  g_ProcessInputSystem = ecs_id(ProcessInputSystem);
  
  ECS_SYSTEM(
      world, BallPaddleCollisions, EcsOnUpdate, Position,
      [inout] BallMovement, [in] Collider, Ball);
  g_BallPaddleCollisions = ecs_id(BallPaddleCollisions);
  
  ECS_SYSTEM(
      world, BallWallCollisions, EcsOnUpdate, Position,
      [inout] BallMovement, [in] Collider, Ball);
  g_BallWallCollisions = ecs_id(BallWallCollisions);
  
  ECS_SYSTEM(
      world, BallPaddleSoundOnCollission, EcsOnUpdate, [in] Position,
      [in] Collider, [in] GameSound, Ball);
  g_BallPaddleSoundOnCollission = ecs_id(BallPaddleSoundOnCollission);
  
  ECS_SYSTEM(
      world, BallWallSoundOnCollission, EcsOnUpdate, [in] Position,
      [in] Collider, [in] GameSound, Ball);
  g_BallWallSoundOnCollission = ecs_id(BallWallSoundOnCollission);
  
  ECS_SYSTEM(
      world, MoveEnemySystem, EcsOnUpdate, [out] Position, Enemy);
  g_MoveEnemySystem = ecs_id(MoveEnemySystem);
  
  ECS_SYSTEM(
      world, UpdateBallFromNetwork, EcsOnUpdate, [out] Position, Ball);
  g_UpdateBallFromNetwork = ecs_id(UpdateBallFromNetwork);
  
  ECS_SYSTEM(
      world, SendPlayerPositionSystem, EcsOnUpdate, [in] Position,
      Player);
  g_SendPlayerPositionSystem = ecs_id(SendPlayerPositionSystem);
}
