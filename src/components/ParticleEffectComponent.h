//
// Created by jack on 3/16/2025.
//

#ifndef PARTICLEEFFECTCOMPONENT_H
#define PARTICLEEFFECTCOMPONENT_H

#include "../world/MaterialName.h"

struct ParticleEffectComponent {
    float footDustTimer = 0.0f;
    MaterialName lastBlockStandingOn = AIR;
    template<class Archive>
    void serialize(Archive& archive)
    {
        //deliberately empty
    }
};

#endif //PARTICLEEFFECTCOMPONENT_H
