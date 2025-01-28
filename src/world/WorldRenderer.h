//
// Created by jack on 1/27/2025.
//

#ifndef WORLDRENDERER_H
#define WORLDRENDERER_H

#include "World.h"
#include "../Camera.h"
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

struct ChangeBuffer
{
    UsableMesh mesh;
    size_t chunkIndex;
    std::atomic<bool> ready = false;  // true when data is ready to be consumed
    std::atomic<bool> in_use = false;
    std::optional<IntTup> from = std::nullopt;
    IntTup to = {};
};

void modifyOrInitializeDrawInstructions(GLuint& vvbo, GLuint& uvvbo, GLuint& ebo, DrawInstructions& drawInstructions, UsableMesh& usable_mesh, GLuint& bvbo);

void drawFromDrawInstructions(const DrawInstructions& drawInstructions);


enum Side
{
    Front = 0,Right,Back,Left,Top,Bottom
};

inline static PxVec3 cubefaces[6][4] = {
    {//front
        PxVec3(0.0, 0.0, 0.0),PxVec3(1.0, 0.0, 0.0),PxVec3(1.0, 1.0, 0.0),PxVec3(0.0, 1.0, 0.0)},
    {//right
        PxVec3(1.0, 0.0, 0.0),PxVec3(1.0, 0.0, 1.0),PxVec3(1.0, 1.0, 1.0),PxVec3(1.0, 1.0, 0.0)},
    {//back
        PxVec3(1.0, 0.0, 1.0),PxVec3(0.0, 0.0, 1.0),PxVec3(0.0, 1.0, 1.0),PxVec3(1.0, 1.0, 1.0)},
    {//left
        PxVec3(0.0, 0.0, 1.0),PxVec3(0.0, 0.0, 0.0),PxVec3(0.0, 1.0, 0.0),PxVec3(0.0, 1.0, 1.0)},
    {//top
        PxVec3(0.0, 1.0, 0.0),PxVec3(1.0, 1.0, 0.0),PxVec3(1.0, 1.0, 1.0),PxVec3(0.0, 1.0, 1.0)},
    {//bottom
        PxVec3(0.0, 0.0, 1.0),PxVec3(1.0, 0.0, 1.0),PxVec3(1.0, 0.0, 0.0),PxVec3(0.0, 0.0, 0.0)}
};

inline static PxU32 ccwindices[6] = {
    0, 1, 2, 2, 3, 0
};

inline static PxU32 cwindices[6] = {
    0, 3, 2, 2, 1, 0
};

inline static IntTup neighborSpots[6] = {
    IntTup(0,0,-1),
    IntTup(1, 0,0),
    IntTup(0,0,1),
    IntTup(-1,0,0),
    IntTup(0,1,0),
    IntTup(0,-1,0)
};


UsableMesh fromChunk(IntTup spot, World* world, int chunkSize);


class WorldRenderer {
public:
    constexpr int chunkSize = 16;
    constexpr int renderDistance = 5;
    constexpr int maxChunks = ((renderDistance*2) * (renderDistance*2)) + renderDistance*5;

    std::vector<ChunkGLInfo> chunkPool;
    std::unordered_map<IntTup, size_t, IntTupHash> activeChunks;

    ///ONLY FOR THE CHUNK THREAD TO ACCESS
    std::unordered_map<IntTup, size_t, IntTupHash> myActiveChunks;

    std::array<ChangeBuffer, 4> changeBuffers;
    boost::lockfree::spsc_queue<size_t, boost::lockfree::capacity<4>> freeChangeBuffers;

    void mainThreadDraw();
    void meshBuildCoroutine(jl::Camera* playerCamera, World* world);

    size_t addChunkBuffer()
    {
        ChunkGLInfo chunk;
        glGenVertexArrays(1, &chunk.drawInstructions.vao);
        glBindVertexArray(chunk.drawInstructions.vao);
        glGenBuffers(1, &chunk.vvbo);
        glGenBuffers(1, &chunk.uvvbo);
        glGenBuffers(1, &chunk.bvbo);
        glGenBuffers(1, &chunk.ebo);

        chunkPool.push_back(chunk);
        return chunkPool.size() - 1;
    }

    IntTup worldToChunkPos(IntTup worldPos)
    {
        return IntTup(
            worldPos.x >= 0 ? worldPos.x / chunkSize : (worldPos.x - chunkSize + 1) / chunkSize,
            worldPos.z >= 0 ? worldPos.z / chunkSize : (worldPos.z - chunkSize + 1) / chunkSize
        );
    }

};



#endif //WORLDRENDERER_H
