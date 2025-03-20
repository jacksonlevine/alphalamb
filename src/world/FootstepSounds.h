//
// Created by jack on 3/19/2025.
//

#ifndef FOOTSTEPSOUNDS_H
#define FOOTSTEPSOUNDS_H


#include "MaterialName.h"
#include "World.h"
#include "../OpenALStuff.h"

constexpr std::optional<SoundBuffers> getFootstepSound(BlockType block)
{
    auto id = static_cast<MaterialName>(block & BLOCK_ID_BITS);
    switch(id)
    {
    case AIR:
        return std::nullopt;
    case STONE:
        return SoundBuffers::JETPACK;
    }
    return SoundBuffers::JETPACK;
}




#endif //FOOTSTEPSOUNDS_H
