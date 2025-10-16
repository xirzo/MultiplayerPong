#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "utils.h"

typedef struct Properties {
    int            SCREEN_WIDTH;
    int            SCREEN_HEIGHT;
    int            FPS_LOCK;
    float          PADDLE_SPEED;
    float          PADDLE_WIDTH;
    float          PADDLE_HEIGHT;
    float          PADDLE_SCREEN_SIZE_MARGIN;
    float          BALL_SIDE;
    float          BALL_MIN_SPEED;
    float          BALL_MAX_SPEED;
    vec2           BALL_INITIAL_DIRECTION;
    float          WALL_THICKNESS;
    float          MIDDLE_LINE_WIDTH;
    char           SERVER_IP[16];
    unsigned short SERVER_PORT;
} Properties;

extern Properties g_Properties;

int properties_load_from_file(const char *filename);

#endif  // !PROPERTIES_H
