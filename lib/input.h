#ifndef INPUT_H
#define INPUT_H

#include <raylib.h>
#include <stddef.h>

#define INPUT_ACTIONS_SIZE ILLEGAL + 1

typedef enum InputAction {
  ACTION_MOVE_UP = 0,
  ACTION_MOVE_DOWN,

  ILLEGAL,
} InputAction;

typedef struct InputBinding {
  InputAction action;
  int key;
  int was_pressed;
  int is_pressed;
} InputBinding;

typedef struct Input {
  InputBinding bindings[INPUT_ACTIONS_SIZE];
} Input;

void InitInput(Input *input);
void UpdateInput(Input *input);
int IsActionPressed(Input *input, InputAction action);
int WasActionPressed(Input *input, InputAction action);

#endif  // !INPUT_H
