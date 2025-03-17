//
// Created by jack on 2/13/2025.
//

#include "VoxModelStampGizmo.h"

#include "../VoxelModels.h"

void VoxModelStampGizmo::draw(World* world, entt::entity playerIndex, entt::registry& reg)
{
    static size_t lastModelIndex = 9999;
    if (active)
    {
        glUseProgram(shaderProgram);
        if (modelIndex != lastModelIndex)
        {
            std::cout << "Model index: " << modelIndex << std::endl;
            UsableMesh mesh = {};
            PxU32 index = 0;
            PxU32 tindex = 0;
            lastModelIndex = modelIndex;
            auto & vm = voxelModels[modelIndex];

            PxVec3 offset = PxVec3(vm.dimensions.x/-2, 0, vm.dimensions.z/-2);
            for (auto& point : vm.points)
            {
                for (int i = 0; i < 6; i++)
                {
                    addFace(offset + PxVec3(point.localSpot.x, point.localSpot.y, point.localSpot.z), (Side)i, (MaterialName)point.colorIndex, 1, mesh, index, tindex);
                }
            }
            modifyOrInitializeDrawInstructions(cgl.vvbo, cgl.uvvbo, cgl.ebo, cgl.drawInstructions, mesh, cgl.bvbo, cgl.tvvbo, cgl.tuvvbo, cgl.tebo, cgl.tbvbo);
        }

        if (cgl.drawInstructions.vao != 0)
        {
            glUniform3f(glGetUniformLocation(shaderProgram, "pos"),
                (float)spot.x, (float)spot.y, (float)spot.z);
            glUniform1f(glGetUniformLocation(shaderProgram, "timeRendered"),
                10.0f);
            drawFromDrawInstructions(cgl.drawInstructions);
            drawTransparentsFromDrawInstructions(cgl.drawInstructions);
        }
    }

}

void VoxModelStampGizmo::init()
{
}
