#ifndef CLIENT_STATE_H_
#define CLIENT_STATE_H_

typedef enum ClientMenu {
  kMainMenu = 0,
  kGame,
} ClientMenu;

typedef struct ClientState {
  ClientMenu current_state;
} ClientState;

void InitClientState(ClientState* s);

#endif  // !CLIENT_STATE_H_
