//
// Created by jack on 3/11/2025.
//

#ifndef GRASSY_H
#define GRASSY_H

#include "SpecialBlockInfo.h"


inline void addGrassy(UsableMesh& mesh, BlockType block, IntTup position, PxU32& index, PxU32& tindex)
{
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

    addShapeWithMaterial(baseGrass, baseGrassBrightness, TALL_GRASS, mesh, position, index, tindex);

}




#endif //GRASSY_H
