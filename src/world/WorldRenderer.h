//
// Created by jack on 1/27/2025.
//

#ifndef WORLDRENDERER_H
#define WORLDRENDERER_H

#include "MaterialName.h"
#include "World.h"
#include "../Camera.h"
#include "../IntTup.h"
#include "../PrecompHeader.h"

struct DrawInstructions
{
    GLuint vao = 0;
    int indiceCount = 0;
    GLuint tvao = 0;
    int tindiceCount = 0;
};

using namespace physx;
struct UsableMesh
{
    std::vector<PxVec3> positions = {};
    std::vector<glm::vec2> texcoords = {};
    std::vector<PxU32> indices = {};
    std::vector<float> brightness = {};

    std::vector<PxVec3> tpositions = {};
    std::vector<glm::vec2> ttexcoords = {};
    std::vector<PxU32> tindices = {};
    std::vector<float> tbrightness = {};
};

struct ChunkGLInfo
{
    GLuint vvbo, uvvbo, bvbo, ebo = 0;
    GLuint tvvbo, tuvvbo, tbvbo, tebo = 0;
    DrawInstructions drawInstructions = {};
};

struct ChangeBuffer
{
    UsableMesh mesh = {};
    size_t chunkIndex = 0;
    std::atomic<bool> ready = false;
    std::atomic<bool> in_use = false;
    std::optional<TwoIntTup> from = std::nullopt;
    TwoIntTup to = {};
};

void modifyOrInitializeDrawInstructions(GLuint& vvbo, GLuint& uvvbo, GLuint& ebo, DrawInstructions& drawInstructions, UsableMesh& usable_mesh, GLuint& bvbo,
    GLuint& tvvbo, GLuint& tuvvbo, GLuint& tebo, GLuint& tbvbo);
void drawTransparentsFromDrawInstructions(const DrawInstructions& drawInstructions);
void drawFromDrawInstructions(const DrawInstructions& drawInstructions);

enum Side
{
    Front = 0,Right,Back,Left,Top,Bottom
};

__inline void addFace(PxVec3 offset, Side side, MaterialName material, int sideHeight, UsableMesh& mesh, uint32_t& index, uint32_t& tindex);

constexpr float onePixel = 0.00183823529411764705882352941176f;     //  1/544      Padding
constexpr float textureWidth = 0.02941176470588235294117647058824f; // 16/544      16 pixel texture width
constexpr float texSlotWidth = 0.03308823529411764705882352941176f;

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


UsableMesh fromChunk(TwoIntTup spot, World* world, int chunkSize);\

struct UsedChunkInfo
{
    bool confirmedByMainThread = false;
    size_t chunkIndex = 0;
    UsedChunkInfo(size_t index) : chunkIndex(index) {}
};


class WorldRenderer {
public:
    static constexpr int chunkSize = 16;
    static constexpr int renderDistance = 16;
    static constexpr int maxChunks = ((renderDistance*2) * (renderDistance*2));
    static constexpr int MIN_DISTANCE = renderDistance + 1;

    std::vector<ChunkGLInfo> chunkPool;


    // Preallocated memory pool for activeChunks and mbtActiveChunks
    std::vector<char> activeChunksMemoryPool;
    std::vector<char> mbtActiveChunksMemoryPool;

    // Monotonic buffer resources for these two hashmaps, because they will only ever have maxchunks size & we want to allocate as infrequently as possible.
    boost::container::pmr::monotonic_buffer_resource activeChunksMemoryResource;
    boost::container::pmr::monotonic_buffer_resource mbtActiveChunksMemoryResource;

    // Polymorphic allocators for the unordered_maps
    using ActiveChunksAllocator = boost::container::pmr::polymorphic_allocator<std::pair<const TwoIntTup, size_t>>;
    using MBTActiveChunksAllocator = boost::container::pmr::polymorphic_allocator<std::pair<const TwoIntTup, UsedChunkInfo>>;

    ///Only for Main Thread access. The main thread will make changes to this as it buffers incoming geometry, so they will only be on this once theyre officially ready to be drawn.
    ///The main thread will also use this as its list of what chunks to draw
    boost::unordered_map<TwoIntTup, size_t, TwoIntTupHash, std::equal_to<TwoIntTup>, ActiveChunksAllocator> activeChunks;

    ///Only for Mesh Building thread access. Its internal record of what spots it has generated & sent out the geometry for.
    boost::unordered_map<TwoIntTup, UsedChunkInfo, TwoIntTupHash, std::equal_to<TwoIntTup>, MBTActiveChunksAllocator> mbtActiveChunks;

    ///Chunks that have had their terrain features generated already.
    std::unordered_set<TwoIntTup, TwoIntTupHash> generatedChunks;
    void generateChunk(World* world, TwoIntTup& chunkSpot);

    WorldRenderer()
        : activeChunksMemoryPool(maxChunks * sizeof(std::pair<TwoIntTup, size_t>)), // 1 MB preallocated pool for activeChunks
          mbtActiveChunksMemoryPool(maxChunks * sizeof(std::pair<TwoIntTup, size_t>)), // 1 MB preallocated pool for mbtActiveChunks
          activeChunksMemoryResource(activeChunksMemoryPool.data(), activeChunksMemoryPool.size()),
          mbtActiveChunksMemoryResource(mbtActiveChunksMemoryPool.data(), mbtActiveChunksMemoryPool.size()),
          activeChunks(&activeChunksMemoryResource),
          mbtActiveChunks(&mbtActiveChunksMemoryResource) {}



    ///A limited list of atomic "Change Buffers" that the mesh building thread can reserve and write to, and the main thread will "check its mail", do the necessary GL calls, and re-free the Change Buffers
    ///by adding its index to freeChangeBuffers.
    std::array<ChangeBuffer, 10> changeBuffers = {};
    ///One way queue, from main thread to mesh building thread, to notify of freed Change Buffers
    boost::lockfree::spsc_queue<size_t, boost::lockfree::capacity<10>> freedChangeBuffers = {};

    ///After being added to mbtActiveChunks, we await a confirmation back in this before we know we can reuse that chunk again
    ///One way queue, from main thread to mesh building thread, to notify of mbtActiveChunks entries that have been confirmed/entered into activeChunks.
    boost::lockfree::spsc_queue<TwoIntTup, boost::lockfree::capacity<128>> confirmedActiveChunksQueue = {};

    void mainThreadDraw(jl::Camera* playerCamera, GLuint shader, WorldGenMethod* worldGenMethod);
    void meshBuildCoroutine(jl::Camera* playerCamera, World* world);

    ///Add a chunk gl info with the vao == 0 (not generated yet). Calling modifyOrInitializeDrawInstructions with it and geometry will initialize it.
    ///Returns the index in chunkPool of the new chunk.
    size_t addUninitializedChunkBuffer()
    {
        ChunkGLInfo chunk;
        chunkPool.push_back(chunk);
        return chunkPool.size() - 1;
    }

    TwoIntTup worldToChunkPos(TwoIntTup worldPos)
    {
        return TwoIntTup(
            std::floor((float)worldPos.x / chunkSize),
            std::floor((float)worldPos.z / chunkSize)
        );
    }

};



#endif //WORLDRENDERER_H
