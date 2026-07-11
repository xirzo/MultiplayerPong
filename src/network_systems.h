#ifndef NETWORK_SYSTEMS_H_
#define NETWORK_SYSTEMS_H_

#include "flecs.h"

void ProcessInputSystem(ecs_iter_t* it);
void MoveEnemySystem(ecs_iter_t* it);
void UpdateBallFromNetwork(ecs_iter_t* it);
void SendPlayerPositionSystem(ecs_iter_t* it);

#endif  // NETWORK_SYSTEMS_H_
