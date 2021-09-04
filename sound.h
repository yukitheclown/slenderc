#ifndef SOUND_DEF
#define SOUND_DEF

#include <AL/al.h>
#include "math.h"

typedef struct {
	ALuint bufferID;
	ALuint sourceID;
} Sound;

void Sound_Play(Sound sound, int loop);
void Sound_Stop(Sound sound);
void Sound_Init();
void Sound_Close();
void Sound_FreeSound(Sound sound);
Sound Sound_CreateSound(const char *filePath, Vec3 pos);
void Sound_SetPosition(Vec3 position, Vec3 direction);
void Sound_SetSoundsPosition(Sound sound, Vec3 position);

#endif