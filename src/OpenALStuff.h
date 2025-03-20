//
// Created by jack on 2/13/2025.
//

#ifndef OPENALSTUFF_H
#define OPENALSTUFF_H

#include "PrecompHeader.h"


#include "Camera.h"

extern ALCdevice* aldevice;
extern ALCcontext* alcontext;

extern std::vector<ALuint> sounds;

void loadSounds();

inline void initOpenAL()
{
    aldevice = alcOpenDevice(NULL); // open default device
    if (aldevice != NULL) {
        alcontext = alcCreateContext(aldevice,NULL); // create context
        if (alcontext != NULL) {
            alcMakeContextCurrent(alcontext); // set active context
        }
    }
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    loadSounds();
}


inline void setListenerToCameraPos(jl::Camera* camera)
{
    auto position = camera->transform.position;
    auto direction = camera->transform.direction;
    auto vel = camera->transform.velocity;
    auto up = camera->transform.up;
    alListener3f(AL_POSITION, position.x, position.y, position.z);
    //std::cout << position.x << ", " << position.y << ", " << position.z << std::endl;
    // OpenAL listener orientation: Forward vector + Up vector
    float orientation[6] = {
        direction.x, direction.y, direction.z,  // Forward direction
        up.x, up.y, up.z                        // Up vector
    };
    alListenerfv(AL_ORIENTATION, orientation);
    alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
    alListenerf(AL_GAIN, 1.0f);
    // if (alGetError() != AL_NO_ERROR)
    // {
    //     std::cout << alGetError() << std::endl;
    // }
}

inline ALuint bufferFromFile(const char* filename) {
    // Open the sound file
    SF_INFO sfInfo;
    SNDFILE* sndFile = sf_open(filename, SFM_READ, &sfInfo);
    if (!sndFile) {
        std::cerr << "Failed to open sound file: " << filename << std::endl;
        return 0;
    }

    // Determine OpenAL format
    ALenum format;
    if (sfInfo.channels == 1)
        format = AL_FORMAT_MONO16;
    else if (sfInfo.channels == 2)
        format = AL_FORMAT_STEREO16;
    else {
        std::cerr << "Unsupported channel count: " << sfInfo.channels << std::endl;
        sf_close(sndFile);
        return 0;
    }

    // Read audio data
    std::vector<short> samples(sfInfo.frames * sfInfo.channels);
    sf_read_short(sndFile, samples.data(), samples.size());
    sf_close(sndFile);

    // Create OpenAL buffer
    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, samples.data(), samples.size() * sizeof(short), sfInfo.samplerate);

    return buffer;
}
inline ALuint makeSource(glm::vec3 position)
{
    ALuint source;
    alGenSources(1, &source);


    // Position the sound in 3D space
    alSource3f(source, AL_POSITION, position.x, position.y, position.z);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f); // No movement
    alSourcei(source, AL_LOOPING, AL_FALSE); // Disable looping

    alSourcef(source, AL_REFERENCE_DISTANCE, 10.0f);
    alSourcef(source, AL_MAX_DISTANCE, 100.0f);
    alSourcef(source, AL_ROLLOFF_FACTOR, 0.5f);

    return source;
}

inline void setSourcePosVel(ALuint source, const glm::vec3& position, const glm::vec3& velocity)
{
    alSource3f(source, AL_POSITION, position.x, position.y, position.z);
    alSource3f(source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

inline void setSourceBuffer(ALuint buffer, ALuint source)
{
    alSourcei(source, AL_BUFFER, buffer);
}


inline void playBufferFromSource(ALuint buffer, ALuint source)
{
    alSourcei(source, AL_BUFFER, buffer);
    alSourcePlay(source);
}








enum class SoundBuffers
{
    DIRTSTEP1,
    DIRTSTEP2,
    DIRTSTEP3,
    DIRTSTEP4,
    GRASSSTEP1,
    GRASSSTEP2,
    GRASSSTEP3,
    GRASSSTEP4,
    GRASSSTEP5,
    GRASSSTEP6,
    JETPACK,
    MULCHSTEP1,
    MULCHSTEP2,
    MULCHSTEP3,
    MULCHSTEP4,
    SANDSTEP1,
    SANDSTEP2,
    SANDSTEP3,
    SANDSTEP4,
    SANDSTEP5,
    STONESTEP1,
    STONESTEP2,
    STONESTEP3,
    STONESTEP4,
    UNDERWATER,
    WATER1,
    WATER2,
    WATER3,
    WATER4,
    WATER5,
    WATERMOVE,
    WOODSTEP1,
    WOODSTEP2,
    WOODSTEP3,
    WOODSTEP4,
    WOODSTEP5
};

enum class SoundBufferSeries
{
    DIRTSTEP,
    GRASSSTEP,
    STONESTEP,
    SANDSTEP,
    WOODSTEP,
    WATERSTEP,
};

std::vector<SoundBuffers>& getBufferSeries(SoundBufferSeries s);

SoundBufferSeries fromMaterial(MaterialName m);

inline void loadSounds()
{
sounds.push_back(bufferFromFile("resources/sfx/dirtstep1.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/dirtstep2.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/dirtstep3.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/dirtstep4.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/grassstep1.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/grassstep2.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/grassstep3.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/grassstep4.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/grassstep5.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/grassstep6.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/jetpack.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/mulchstep1.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/mulchstep2.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/mulchstep3.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/mulchstep4.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/sandstep1.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/sandstep2.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/sandstep3.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/sandstep4.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/sandstep5.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/stonestep1.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/stonestep2.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/stonestep3.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/stonestep4.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/underwater.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/water1.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/water2.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/water3.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/water4.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/water5.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/watermove.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/woodstep1.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/woodstep2.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/woodstep3.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/woodstep4.mp3"));
    sounds.push_back(bufferFromFile("resources/sfx/woodstep5.mp3"));
}

#endif //OPENALSTUFF_H


