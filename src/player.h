#ifndef PLAYER_H_
#define PLAYER_H_

#include "flecs.h"
#include "input.h"

typedef struct {
  bool is_main;
} Player;

typedef struct PlayerInput {
  InputAction up_action;
  InputAction down_action;
  Input* input;
} PlayerInput;

#endif  // PLAYER_H_
