#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "utils.h"

typedef struct Properties {
  const int SCREEN_WIDTH;
  const int SCREEN_HEIGHT;
  const int FPS_LOCK;
  const float PADDLE_SPEED;
  const float PADDLE_WIDTH;
  const float PADDLE_HEIGHT;
  const float PADDLE_SCREEN_SIZE_MARGIN;
  const float BALL_SIDE;
  const float BALL_MIN_SPEED;
  const float BALL_MAX_SPEED;
  const vec2 BALL_INITIAL_DIRECTION;
  const float WALL_THICKNESS;
  const float MIDDLE_LINE_WIDTH;
  const char *SERVER_IP;
  const unsigned short SERVER_PORT;
} Properties;

static inline const Properties *get_properties(void) {
  static const Properties g_Properties = {
      .SCREEN_WIDTH = 900,
      .SCREEN_HEIGHT = 600,
      .FPS_LOCK = 60,
      .PADDLE_SPEED = 450.f,
      .PADDLE_WIDTH = 20.f,
      .PADDLE_HEIGHT = 80.f,
      .PADDLE_SCREEN_SIZE_MARGIN = 50.f,
      .BALL_SIDE = 10.f,
      .BALL_MIN_SPEED = 450.f,
      .BALL_MAX_SPEED = 650.f,
      .BALL_INITIAL_DIRECTION = {0.7f, -0.7f},
      .WALL_THICKNESS = 10.f,
      .MIDDLE_LINE_WIDTH = 10.f,
      .SERVER_IP = "127.0.0.1",
      .SERVER_PORT = 8080,
  };
  return &g_Properties;
}

#define g_Properties (*get_properties())

#endif // !PROPERTIES_H
