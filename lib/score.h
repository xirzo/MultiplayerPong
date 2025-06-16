#ifndef SCORE_H
#define SCORE_H

#include "flecs.h"

typedef struct Score {
  int value;
} Score;

void ScoreCountSystem(ecs_iter_t *it);

#endif // !SCORE_H
