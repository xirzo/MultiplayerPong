#ifndef PROPERTIES_H
#define PROPERTIES_H

typedef struct Properties {
  int SCREEN_WIDTH;
  int SCREEN_HEIGHT;
  int FPS_LOCK;
  float PADDLE_SPEED;
  float PADDLE_WIDTH;
  float PADDLE_HEIGHT;
  float PADDLE_SCREEN_SIZE_MARGIN;
  float BALL_SIDE;
  float WALL_THICKNESS;
} Properties;

#endif // !PROPERTIES_H
