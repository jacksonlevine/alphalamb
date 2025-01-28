//
// Created by jack on 1/27/2025.
//

#ifndef WORLDRENDERER_H
#define WORLDRENDERER_H

#include "../IntTup.h"
#include "../PrecompHeader.h"

struct DrawInstructions
{
    GLuint vao = 0;
    int indiceCount = 0;
};

using namespace physx;
struct UsableMesh
{
    std::vector<PxVec3> positions = {};
    std::vector<glm::vec2> texcoords = {};
    std::vector<PxU32> indices = {};
    std::vector<float> brightness = {};
};

struct ChunkGLInfo
{
    GLuint vvbo, uvvbo, bvbo, ebo = 0;
    DrawInstructions drawInstructions = {};
};

void modifyOrInitializeDrawInstructions(GLuint& vvbo, GLuint& uvvbo, GLuint& ebo, DrawInstructions& drawInstructions, UsableMesh& usable_mesh, GLuint& bvbo);

void drawFromDrawInstructions(const DrawInstructions& drawInstructions);


class WorldRenderer {
public:
    constexpr int renderDistance = 5;
    constexpr int maxChunks = ((renderDistance*2) * (renderDistance*2)) + renderDistance*5;

    std::vector<ChunkGLInfo> chunkPool;
    std::unordered_map<IntTup, size_t, IntTupHash> activeChunks;

    ///ONLY FOR THE CHUNK THREAD TO ACCESS
    std::unordered_map<IntTup, size_t, IntTupHash> myActiveChunks;

    size_t addChunkBuffer()
    {
        ChunkGLInfo chunk;
        glGenVertexArrays(1, &chunk.drawInstructions.vao);
        glBindVertexArray(chunk.drawInstructions.vao);
        glGenBuffers(1, &chunk.vvbo);
        glGenBuffers(1, &chunk.uvvbo);
        glGenBuffers(1, &chunk.bvbo);
        glGenBuffers(1, &chunk.ebo);
    }

};



#endif //WORLDRENDERER_H
