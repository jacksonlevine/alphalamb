//
// Created by jack on 2/13/2025.
//

#include "OpenALStuff.h"

ALCdevice* aldevice = nullptr;
ALCcontext* alcontext = nullptr;

std::vector<ALuint> sounds;

#include "world/MaterialName.h"

SoundBufferSeries fromMaterial(MaterialName m)
{
    switch(m)
    {
    case WOOD: case WOOD_PLANKS: case WOODEN_TRUNK: case WOOD_STAIRS:
        return SoundBufferSeries::WOODSTEP;
    case DIRT:
        return SoundBufferSeries::DIRTSTEP;
    case SAND: case SNOWY_SAND:
        return SoundBufferSeries::SANDSTEP;
    case GRASS: case TALL_GRASS: case BAMBOO:
        return SoundBufferSeries::GRASSSTEP;
    case WATER:
        return SoundBufferSeries::WATERSTEP;
    default:
        return SoundBufferSeries::STONESTEP;
    }
}

std::vector<SoundBuffers>& getBufferSeries(SoundBufferSeries s)
{
    switch(s)
    {
    case SoundBufferSeries::DIRTSTEP:
        {
            static std::vector<SoundBuffers> sounds = {
                SoundBuffers::DIRTSTEP1,
                SoundBuffers::DIRTSTEP2,
                SoundBuffers::DIRTSTEP3,
                SoundBuffers::DIRTSTEP4,
            };
            return sounds;
        }
    case SoundBufferSeries::GRASSSTEP:
        {
            static std::vector<SoundBuffers> sounds = {
                SoundBuffers::GRASSSTEP1,
                SoundBuffers::GRASSSTEP2,
                SoundBuffers::GRASSSTEP3,
                SoundBuffers::GRASSSTEP4,
                SoundBuffers::GRASSSTEP5,
                SoundBuffers::GRASSSTEP6,
            };
            return sounds;
        }
    case SoundBufferSeries::STONESTEP:
        {
            static std::vector<SoundBuffers> sounds = {
                SoundBuffers::STONESTEP1,
                SoundBuffers::STONESTEP2,
                SoundBuffers::STONESTEP3,
                SoundBuffers::STONESTEP4,
            };
            return sounds;
        }
    case SoundBufferSeries::SANDSTEP:
        {
            static std::vector<SoundBuffers> sounds = {
                SoundBuffers::SANDSTEP1,
                SoundBuffers::SANDSTEP2,
                SoundBuffers::SANDSTEP3,
                SoundBuffers::SANDSTEP4,
                SoundBuffers::SANDSTEP5,
            };
            return sounds;
        }
    case SoundBufferSeries::WOODSTEP:
        {
            static std::vector<SoundBuffers> sounds = {
                SoundBuffers::WOODSTEP1,
                SoundBuffers::WOODSTEP2,
                SoundBuffers::WOODSTEP3,
                SoundBuffers::WOODSTEP4,
                SoundBuffers::WOODSTEP5,
            };
            return sounds;
        }
    case SoundBufferSeries::WATERSTEP:
        {
            static std::vector<SoundBuffers> sounds = {
                SoundBuffers::WATER1,
                SoundBuffers::WATER2,
                SoundBuffers::WATER3,
                SoundBuffers::WATER4,
                SoundBuffers::WATER5,
            };
            return sounds;
        }
    default:
        {
            static std::vector<SoundBuffers> empty = {};
            return empty;
        }
    }

}