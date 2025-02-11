//
// Created by jack on 2/10/2025.
//

#ifndef HANDLEDBLOCK_H
#define HANDLEDBLOCK_H




#include "PrecompHeader.h"
#include "world/MaterialName.h"
#include "world/WorldRenderer.h"

//Player must provide current block, a last held block storage variable, and a ChunkGLInfo storage variable
//Uniform the pos of the player first.
inline void drawHandledBlock(glm::vec3 playerPos, MaterialName block, GLuint gltfShader, MaterialName& lastHeldBlock, ChunkGLInfo& meshInfo)
{



    if (block != lastHeldBlock)
    {
        UsableMesh mesh = {};
        PxU32 index = 0;
        PxU32 tindex = 0;
        for (int i = 0; i < 6; i++)
        {
            addFace(PxVec3(0.0), (Side)i, block, 1, mesh, index, tindex);
        }
        modifyOrInitializeDrawInstructions(meshInfo.vvbo, meshInfo.uvvbo, meshInfo.ebo, meshInfo.drawInstructions, mesh, meshInfo.bvbo,
            meshInfo.tvvbo, meshInfo.tuvvbo, meshInfo.tebo, meshInfo.tbvbo);
        lastHeldBlock = block;
    }
    if (meshInfo.drawInstructions.vao != 0)
    {
        if (std::find(transparents.begin(), transparents.end(), block) != transparents.end())
        {
            drawTransparentsFromDrawInstructions(meshInfo.drawInstructions);
        } else
        {
            drawFromDrawInstructions(meshInfo.drawInstructions);
        }

    }


}



#endif //HANDLEDBLOCK_H
