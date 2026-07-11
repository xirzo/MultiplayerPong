#ifndef COMPONENTS_H_
#define COMPONENTS_H_

#include "flecs.h"

extern ECS_COMPONENT_DECLARE(Position);
extern ECS_COMPONENT_DECLARE(BallMovement);
extern ECS_COMPONENT_DECLARE(PaddleMovement);
extern ECS_COMPONENT_DECLARE(MovementClamp);
extern ECS_COMPONENT_DECLARE(RenderableRectangle);
extern ECS_COMPONENT_DECLARE(PlayerInput);
extern ECS_COMPONENT_DECLARE(Collider);
extern ECS_COMPONENT_DECLARE(Score);
extern ECS_COMPONENT_DECLARE(GameSound);
extern ECS_COMPONENT_DECLARE(Player);

extern ECS_TAG_DECLARE(Ball);
extern ECS_TAG_DECLARE(Paddle);
extern ECS_TAG_DECLARE(Enemy);
extern ECS_TAG_DECLARE(Wall);

#endif  // COMPONENTS_H_
