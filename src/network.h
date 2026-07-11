#ifndef NETWORK_H_
#define NETWORK_H_

#include "game_state.h"
#include "server.h"

void HandleServerMessage(GameState* state, const ServerMessage* msg);

#endif  // NETWORK_H_
