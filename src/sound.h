#ifndef SOUND_H
#define SOUND_H

#include <raylib.h>
#include "flecs.h"

typedef struct {
  Sound sound;
} GameSound;

void BallPaddleSoundOnCollission(ecs_iter_t* it);
void BallWallSoundOnCollission(ecs_iter_t* it);

#endif  // !SOUND_H
