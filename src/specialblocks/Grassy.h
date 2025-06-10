//
// Created by jack on 3/11/2025.
//

#ifndef GRASSY_H
#define GRASSY_H

#include "SpecialBlockInfo.h"


inline void addGrassy(UsableMesh& mesh, BlockType block, IntTup position, PxU32& index, PxU32& tindex)
{

    static FastNoiseLite* perl = new FastNoiseLite();
    static bool initted = false;
    if (!initted)
    {
        initted = true;
        perl->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    }
    static std::vector<PxVec3> baseGrass = {
        PxVec3(0.146f, 0.0f, 0.146f),
            PxVec3(0.854f, 0.0f, 0.854f),
            PxVec3(0.854f, 1.0f, 0.854f),
            PxVec3(0.146f, 1.0f, 0.146f),

            PxVec3(0.854f, 0.0f, 0.854f),
            PxVec3(0.146f, 0.0f, 0.146f),
            PxVec3(0.146f, 1.0f, 0.146f),
            PxVec3(0.854f, 1.0f, 0.854f),

            PxVec3(0.854f, 0.0f, 0.146f),
            PxVec3(0.146f, 0.0f, 0.854f),
            PxVec3(0.146f, 1.0f, 0.854f),
            PxVec3(0.854f, 1.0f, 0.146f),

            PxVec3(0.146f, 0.0f, 0.854f),
            PxVec3(0.854f, 0.0f, 0.146f),
            PxVec3(0.854f, 1.0f, 0.146f),
            PxVec3(0.146f, 1.0f, 0.854f),
    };

    static std::vector baseGrassBrightness = {

        ColorPack((uint8_t)15u),
        ColorPack((uint8_t)15u),
        ColorPack((uint8_t)14u),
        ColorPack((uint8_t)14u),
    };

    float offsetx = perl->GetNoise(position.x*50.5f, position.z*50.5f)*0.5f;
    float offsetz = perl->GetNoise(position.x*50.5f + 100.f, position.z*50.5f)*0.5f;
    addShapeWithMaterial(baseGrass, baseGrassBrightness, TALL_GRASS, mesh, PxVec3(position.x + offsetx, position.y, position.z + offsetz), index, tindex);

}




#endif //GRASSY_H
