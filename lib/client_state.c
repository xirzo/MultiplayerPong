#include "client_state.h"

void InitClientState(ClientState* s) {
  s->current_state = kMainMenu;
}
