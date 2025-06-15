#include "input.h"

void InitInput(Input *input) {
  for (size_t i = 0; i < ILLEGAL; i++) {
    input->bindings[i] = (InputBinding){ILLEGAL, 0, 0, 0};
  }

  input->bindings[ACTION_MOVE_UP] =
      (InputBinding){.action = ACTION_MOVE_UP, .key = KEY_W, .is_pressed = 0, .was_pressed = 0};
  input->bindings[ACTION_MOVE_DOWN] =
      (InputBinding){.action = ACTION_MOVE_DOWN, .key = KEY_S, .is_pressed = 0, .was_pressed = 0};
}

void UpdateInput(Input *input) {
  for (size_t i = 0; i < ILLEGAL; i++) {
    InputBinding *binding = &input->bindings[i];
    binding->was_pressed = binding->is_pressed;
    binding->is_pressed = IsKeyDown(binding->key);
  }
}

int IsActionPressed(Input *input, InputAction action) {
  if (action < 0 || action >= INPUT_ACTIONS_SIZE) {
    return 0;
  }

  return input->bindings[action].is_pressed;
}

int WasActionPressed(Input *input, InputAction action) {
  if (action < 0 || action >= INPUT_ACTIONS_SIZE) {
    return 0;
  }

  return input->bindings[action].was_pressed;
}
