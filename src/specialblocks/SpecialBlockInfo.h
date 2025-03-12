//
// Created by jack on 3/10/2025.
//

#ifndef SPECIALBLOCKINFO_H
#define SPECIALBLOCKINFO_H

#include "../world/WorldRenderer.h"

using addBlockFunc = std::function<void(UsableMesh&, uint32_t, IntTup, PxU32 &, PxU32 &)>;

using setBitsFunc = std::function<void(World*, IntTup)>;

inline void addShapeWithMaterial(std::vector<PxVec3> &cwtriangles, MaterialName block, UsableMesh& outmesh, IntTup position, PxU32& index, PxU32& tindex)
{

    auto & tex = TEXS[(int)block];
    auto & face = tex[0];

    float uvoffsetx = static_cast<float>(face.first) * texSlotWidth;
    float uvoffsety = 1.0f - (static_cast<float>(face.second) * texSlotWidth);

    static glm::vec2 texOffsets[] = {
        glm::vec2(onePixel, -onePixel),
        glm::vec2(onePixel + textureWidth, -onePixel),
        glm::vec2(onePixel + textureWidth, -(onePixel + textureWidth)),
        glm::vec2(onePixel, -(onePixel + textureWidth)),
    };
    //If the material's transparent, add these verts to the "t____" parts of the mesh. If not, add them to the normal mesh.
    PxVec3 offset(position.x, position.y, position.z);

    if (std::ranges::find(transparents, block) != transparents.end())
    {
        auto transformed =  cwtriangles | std::views::transform([&](const PxVec3& v) {
            auto newv = v;
            return newv + offset;
        }) | std::ranges::to<std::vector>();

        outmesh.tpositions.insert(outmesh.tpositions.end(), transformed.begin(), transformed.end());

        for (int i = 0; i < cwtriangles.size()/4; i++)
        {
            std::ranges::transform(ccwindices, std::back_inserter(outmesh.tindices), [tindex](const auto& i) {
                return tindex + i;
            });
            tindex += 4;
        }

        for (int i = 0; i < cwtriangles.size()/4; i++)
        {
            outmesh.ttexcoords.insert(outmesh.ttexcoords.end(),{
                glm::vec2(uvoffsetx + texOffsets[0].x, uvoffsety + texOffsets[0].y),
            glm::vec2(uvoffsetx + texOffsets[1].x, uvoffsety + texOffsets[1].y),
            glm::vec2(uvoffsetx + texOffsets[2].x, uvoffsety + texOffsets[2].y),
            glm::vec2(uvoffsetx + texOffsets[3].x, uvoffsety + texOffsets[3].y),
            });
        }


        float isGrass = block == GRASS ? 1.0f : 0.0f;

        for (int i = 0; i < cwtriangles.size()/4; i++)
        {
            outmesh.tbrightness.insert(outmesh.tbrightness.end(), {1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass});
        }


    } else
    {
        auto transformed =  cwtriangles | std::views::transform([&](const PxVec3& v) {
             auto newv = v;
             return newv + offset;
         }) | std::ranges::to<std::vector>();

        outmesh.positions.insert(outmesh.positions.end(), transformed.begin(), transformed.end());

        for (int i = 0; i < cwtriangles.size()/4; i++)
        {
            std::ranges::transform(ccwindices, std::back_inserter(outmesh.indices), [index](const auto& i) {
                return index + i;
            });
            index += 4;
        }

        for (int i = 0; i < cwtriangles.size()/4; i++)
        {
            outmesh.texcoords.insert(outmesh.texcoords.end(),{
                glm::vec2(uvoffsetx + texOffsets[0].x, uvoffsety + texOffsets[0].y),
            glm::vec2(uvoffsetx + texOffsets[1].x, uvoffsety + texOffsets[1].y),
            glm::vec2(uvoffsetx + texOffsets[2].x, uvoffsety + texOffsets[2].y),
            glm::vec2(uvoffsetx + texOffsets[3].x, uvoffsety + texOffsets[3].y),
            });
        }


        float isGrass = block == GRASS ? 1.0f : 0.0f;

        for (int i = 0; i < cwtriangles.size()/4; i++)
        {
            outmesh.brightness.insert(outmesh.brightness.end(), {1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass});
        }

    }
}

inline std::vector<physx::PxVec3> rotateCoordinatesAroundYNegative90(const std::vector<physx::PxVec3>& coords, int32_t numRotations) {
    std::vector<physx::PxVec3> rotatedCoords = coords;

    const physx::PxVec3 center(0.5f, 0.5f, 0.5f);

    for (int32_t rotation = 0; rotation < numRotations; ++rotation) {
        for (auto& vec : rotatedCoords) {
            // Translate to origin
            physx::PxVec3 translated = vec - center;

            // Perform 90-degree clockwise rotation around Y axis
            // x' = -z, z' = x
            physx::PxVec3 rotated;
            rotated.x = -translated.z;
            rotated.y = translated.y;
            rotated.z = translated.x;

            // Translate back
            vec = rotated + center;
        }
    }

    return rotatedCoords;
}

#endif //SPECIALBLOCKINFO_H
