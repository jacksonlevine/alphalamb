//
// Created by jack on 6/17/2025.
//

#ifndef SOUNDSOURCE_H
#define SOUNDSOURCE_H


#include "../OpenALStuff.h"
#include "../PrecompHeader.h"

struct Orange1Hisser{};
struct PlayerSounds{};


template <typename T>
struct SoundSource
{
    ALuint source = 0;
    SoundSource()
    {
        source = makeSource(glm::vec3(0.f));
    }
    void setPos(const glm::vec3& pos, const glm::vec3& vel = glm::vec3(0.f))
    {
        alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
        alSource3f(source, AL_VELOCITY, vel.x, vel.y, vel.z);
    }
    void stop()
    {
        ALint val;
        alGetSourcei(source, AL_SOURCE_STATE, &val);
        if (val == AL_STOPPED) return;
        alSourceStop(source);
    }
    void play(ALuint buffer)
    {
        stop();
        alSourcef(source, AL_PITCH, 1.0f + ((float)rand() / (float) RAND_MAX) * 0.5f);
        alSourcei(source, AL_BUFFER, buffer);
        alSourcePlay(source);
    }

    ~SoundSource()
    {
        deleteSource(source);
        source = 0;
    }
    SoundSource(const SoundSource&) = delete;
    SoundSource& operator=(const SoundSource&) = delete;
    SoundSource(SoundSource&& other) = delete;
    SoundSource& operator=(SoundSource&& other) = delete;
};

#endif //SOUNDSOURCE_H
