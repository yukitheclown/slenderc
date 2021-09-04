#include "sound.h"
#include <stdio.h>
#include <string.h>
#include <AL/alut.h>
#include <stdlib.h>
#include <vorbis/vorbisfile.h>

#define BUFFER_SIZE 32768 // 32 KB buffers

void Sound_Init(){

    alutInit(0, NULL);
    alGetError();

    alListener3f(AL_POSITION, 0, 0, 0);
}

void Sound_Close(){

    alutExit();
}

static void LoadOgg(const char *path, char **buffer, int *bufferSize, ALenum *format, ALsizei *freq){

    FILE *fp = fopen(path, "rb");

    if(!fp) return;

    *bufferSize = 0;

    int bitStream = 0;
    long bytes = 0;
    char array[BUFFER_SIZE];

    OggVorbis_File oggFile;

    if(ov_open(fp, &oggFile, NULL, 0) != 0){
        printf("Error loading %s\n", path);
        return;
    }

    vorbis_info *pInfo = ov_info(&oggFile, -1);

    if(pInfo->channels == 1)
        *format = AL_FORMAT_MONO16;
    else
        *format = AL_FORMAT_STEREO16;

    *freq = pInfo->rate;

    do {

        bytes = ov_read(&oggFile, array, BUFFER_SIZE, 0, 2, 1, &bitStream);

        if (bytes < 0){
            ov_clear(&oggFile);
            printf("Error loading %s\n", path);
            return;
        }

        *bufferSize += bytes;

        *buffer = (char *)realloc(*buffer, *bufferSize); 
        memcpy(&(*buffer)[(*bufferSize)-bytes], array, bytes);

    } while(bytes > 0);

    ov_clear(&oggFile);
}

void Sound_FreeSound(Sound sound){
    alDeleteBuffers(1, &sound.bufferID);
    alDeleteBuffers(1, &sound.sourceID);
}

static void ALCheckError(){

    ALenum error = alGetError();

    if(error == AL_INVALID_NAME)
        printf("AL_INVALID_NAME\n");
    else if(error == AL_INVALID_ENUM)
        printf("AL_INVALID_ENUM\n");
    else if(error == AL_INVALID_VALUE)
        printf("AL_INVALID_VALUE\n");
    else if(error == AL_INVALID_OPERATION)
        printf("AL_INVALID_OPERATION\n");
    else if(error == AL_OUT_OF_MEMORY)
        printf("AL_OUT_OF_MEMORY\n");
}

Sound Sound_CreateSound(const char *filePath, Vec3 pos){

    Sound sound;
    memset(&sound, 0, sizeof(Sound));

    ALenum format;
    ALsizei freq;

    alGenBuffers(1, &sound.bufferID);
    alGenSources(1, &sound.sourceID);

    ALCheckError();

    alSource3f(sound.sourceID, AL_POSITION, pos.x, pos.y, pos.z);

    char *data = NULL;
    int size = 0;
    LoadOgg(filePath, &data, &size, &format, &freq);

    if(!data) return sound;

    alBufferData(sound.bufferID, format, &data[0], (ALsizei)size, freq);

    free(data);

    alSourcei(sound.sourceID, AL_BUFFER, sound.bufferID);

    return sound;
}

void Sound_Play(Sound sound, int loop){

    int status;

    alGetSourcei(sound.sourceID, AL_SOURCE_STATE, &status);

    if(status == AL_PLAYING) return;

    alSourcePlay(sound.sourceID);
    alSourcei(sound.sourceID, AL_LOOPING, loop);
}

void Sound_Stop(Sound sound){
    alSourceStop(sound.sourceID);
}

void Sound_SetPosition(Vec3 position, Vec3 direction){

    alListener3f(AL_POSITION, position.x, position.y, position.z);

    float directionVect[] = {
        direction.x,
        direction.y,
        direction.z,
        0,
        1,
        0
    };

    alListenerfv(AL_ORIENTATION, directionVect);
}

void Sound_SetSoundsPosition(Sound sound, Vec3 position){
    alSource3f(sound.sourceID,AL_POSITION, position.x, position.y, position.z);
}