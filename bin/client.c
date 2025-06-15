#include "server.h"
#include <stdio.h>

int main() {
  Client *client = sr_client_create("127.0.0.1", 8080);

  if (!client) {
    fprintf(stderr, "error: Failed to create client\n");
    return 1;
  }

  vec2 pos = {10.5f, 20.3f};

  sr_send_client_position(client, pos);

  sr_client_destroy(client);
  return 0;
}
