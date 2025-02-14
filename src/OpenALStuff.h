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
    JETPACK
};

inline void loadSounds()
{
    sounds.push_back(bufferFromFile("resources/sfx/jetpack.mp3"));
}

#endif //OPENALSTUFF_H


