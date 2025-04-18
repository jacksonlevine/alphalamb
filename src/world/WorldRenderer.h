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
#include "RebuildQueue.h"
#include "../Light.h"



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
    GLuint vvbo{}, uvvbo{}, bvbo{}, ebo = 0;
    GLuint tvvbo{}, tuvvbo{}, tbvbo{}, tebo = 0;
    DrawInstructions drawInstructions = {};
};

struct SmallChunkGLInfo
{
    int indiceCount = 0;
    int tindiceCount = 0;
};

struct ChangeBuffer
{
    UsableMesh mesh = {};
    size_t chunkIndex = 0;
    std::optional<TwoIntTup> from = std::nullopt;
    TwoIntTup to = {};
    std::atomic<bool> ready = false;
    std::atomic<bool> in_use = false;
};




void modifyOrInitializeDrawInstructions(GLuint& vvbo, GLuint& uvvbo, GLuint& ebo, DrawInstructions& drawInstructions, UsableMesh& usable_mesh, GLuint& bvbo,
    GLuint& tvvbo, GLuint& tuvvbo, GLuint& tebo, GLuint& tbvbo);
inline void modifyOrInitializeChunkGLInfo(ChunkGLInfo& info, UsableMesh& mesh)
{
    modifyOrInitializeDrawInstructions(info.vvbo, info.uvvbo, info.ebo, info.drawInstructions, mesh, info.bvbo, info.tvvbo, info.tuvvbo, info.tebo, info.tbvbo);
}
void modifyOrInitializeChunkIndex(int chunkIndex, SmallChunkGLInfo& info, UsableMesh& usable_mesh);
void drawTransparentsFromDrawInstructions(const DrawInstructions& drawInstructions);
void drawFromDrawInstructions(const DrawInstructions& drawInstructions);

void drawFromChunkIndex(int chunkIndex, const SmallChunkGLInfo& cgl);
void drawTransparentsFromChunkIndex(int chunkIndex, const SmallChunkGLInfo& cgl);


enum class Side
{
    Front = 0,Right,Back,Left,Top,Bottom
};
// inline static IntTup neighborSpots[6] = {
//     IntTup(0,0,-1),
//     IntTup(1, 0,0),
//     IntTup(0,0,1),
//     IntTup(-1,0,0),
//     IntTup(0,1,0),
//     IntTup(0,-1,0)
// };

template<bool doBrightness = true>
__inline void addFace(PxVec3 offset, Side side, MaterialName material, int sideHeight, UsableMesh& mesh, uint32_t& index, uint32_t& tindex, float offsety = 0.0f);

void genCGLBuffers();
constexpr float onePixel = 0.00183823529411764705882352941176f;     //  1/544      Padding
constexpr float textureWidth = 0.02941176470588235294117647058824f; // 16/544      16 pixel texture width
constexpr float texSlotWidth = 0.03308823529411764705882352941176f;
std::array<IntTup, 3> getAdjacentOffsets(Side side, int vertexIndex);
void calculateAmbientOcclusion(const IntTup& blockPos, Side side, World* world, bool locked, BlockType blockType, UsableMesh& mesh, float
                               blockandambbright);
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
template<bool queueimplics = true>
UsableMesh fromChunk(const TwoIntTup& spot, World* world, int chunkSize, bool locked, bool light);

UsableMesh fromChunk(const TwoIntTup& spot, World* world, int chunkSize, bool light = false);

UsableMesh fromChunkLocked(const TwoIntTup& spot, World* world, int chunkSize, bool light = false);

struct UsedChunkInfo
{
    bool confirmedByMainThread = false;
    size_t chunkIndex = 0;
    explicit UsedChunkInfo(size_t index) : chunkIndex(index) {}
};

struct ReadyToDrawChunkInfo
{
    size_t chunkIndex = 0;
    float timeBeenRendered = 0.0f;
    explicit ReadyToDrawChunkInfo(size_t index) : chunkIndex(index) {}
};
extern std::atomic<int> NUM_THREADS_RUNNING;

extern LightMapType lightmap;
extern LightMapType ambientlightmap;
extern std::shared_mutex lightmapMutex;
extern tbb::concurrent_unordered_set<TwoIntTup, TwoIntTupHash> litChunks;

class WorldRenderer {
public:
    static constexpr int chunkSize = 16;
    static constexpr int renderDistance = 17;
    int currentRenderDistance = 17;
    static constexpr int maxChunks = ((renderDistance*2) * (renderDistance*2));
    int MIN_DISTANCE = renderDistance + 1;
    int lastMaxChunks = maxChunks;


    int currentMinDistance()
    {
        return currentRenderDistance + 1;
    }
    int currentMaxChunks()
    {
        return (currentRenderDistance*2) * (currentRenderDistance*2);
    }



    std::array<SmallChunkGLInfo, maxChunks> chunkPool;

    std::atomic<size_t> chunkPoolSize = {0};

    RebuildQueue rebuildQueue;
    std::thread rebuildThread;
    std::thread chunkWorker;
    std::atomic<int> rebuildThreadRunning = false;
    std::atomic<int> meshBuildingThreadRunning = false;



    //For the user
    std::mutex bufferMutex = {};
    std::condition_variable bufferCV = {};

    //For the mbt
    std::mutex mbtBufferMutex = {};
    std::condition_variable mbtBufferCV = {};


    void notifyBufferFreed() {
        bufferCV.notify_one();
    }

    ///Only for Main Thread access. The main thread will make changes to this as it buffers incoming geometry, so they will only be on this once theyre officially ready to be drawn.
    ///The main thread will also use this as its list of what chunks to draw
    boost::unordered_flat_map<TwoIntTup, ReadyToDrawChunkInfo, TwoIntTupHash, std::equal_to<>> activeChunks = {};

    ///Only for Mesh Building thread access. Its internal record of what spots it has generated & sent out the geometry for.
    boost::unordered_flat_map<TwoIntTup, UsedChunkInfo, TwoIntTupHash, std::equal_to<>> mbtActiveChunks = {};

    ///Chunks that have had their terrain features generated already.
    boost::unordered_flat_set<TwoIntTup, TwoIntTupHash> generatedChunks;

    static void generateChunk(World* world, const TwoIntTup& chunkSpot, std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks = nullptr);

    WorldRenderer()
    {
        mbtActiveChunks.reserve(maxChunks);
        activeChunks.reserve(maxChunks);
        //Generate them all at the beginning so we know the buffer names for sure
        std::ranges::for_each(std::views::iota(0, maxChunks), [](auto) {
            genCGLBuffers();
        });
    }

    void clearInFlightMeshUpdates();

    void setRenderDistance(int newone, jl::Camera* camera, World* world)
    {

        stopThreads();

        lastMaxChunks = currentMaxChunks();
        currentRenderDistance = newone;

        if (lastMaxChunks > currentMaxChunks())
        {
            for (int i = lastMaxChunks-1; i > currentMaxChunks(); i--)
            {
                auto sgl = SmallChunkGLInfo{};
                auto mesh = UsableMesh{};
                modifyOrInitializeChunkIndex(i, sgl, mesh);
            }
        }
        chunkPoolSize.store(0);
        activeChunks.clear();
        mbtActiveChunks.clear();

        clearInFlightMeshUpdates();


        launchThreads(camera, world);




    }



    ///A limited list of atomic "Change Buffers" that the mesh building thread can reserve and write to, and the main thread will "check its mail", do the necessary GL calls, and re-free the Change Buffers
    ///by adding its index to freeChangeBuffers.
    std::array<ChangeBuffer, 64> changeBuffers = {};
    ///One way queue, from main thread to mesh building thread, to notify of freed Change Buffers
    boost::lockfree::spsc_queue<size_t, boost::lockfree::capacity<64>> freedChangeBuffers = {};


    ///A limited list of atomic "Change Buffers" that the mesh building thread can reserve and write to, and the main thread will "check its mail", do the necessary GL calls, and re-free the Change Buffers
    ///by adding its index to freeChangeBuffers.
    std::array<ChangeBuffer, 64> userChangeMeshBuffers = {};
    ///One way queue, from main thread to mesh building thread, to notify of freed Change Buffers
    boost::lockfree::spsc_queue<size_t, boost::lockfree::capacity<64>> freedUserChangeMeshBuffers = {};

    ///After being added to mbtActiveChunks, we await a confirmation back in this before we know we can reuse that chunk again
    ///One way queue, from main thread to mesh building thread, to notify of mbtActiveChunks entries that have been confirmed/entered into activeChunks.
    boost::lockfree::spsc_queue<TwoIntTup, boost::lockfree::capacity<64>> confirmedActiveChunksQueue = {};



    //A way for the rebuild thread to ask the main thread "What chunks are active from this area?" so that the main thread can submit rebuild requests for them
    boost::lockfree::spsc_queue<BlockArea, boost::lockfree::capacity<128>> rebuildToMainAreaNotifications = {};


    double getDewyFogFactor(float temperature_noise, float humidity_noise);
    void mainThreadDraw(const jl::Camera* playerCamera, GLuint shader, WorldGenMethod* worldGenMethod, float deltaTime, bool actuallyDraw);
    void meshBuildCoroutine(jl::Camera* playerCamera, World* world);

    ///Add a chunk gl info with the vao == 0 (not generated yet). Calling modifyOrInitializeDrawInstructions with it and geometry will initialize it.
    ///Returns the index in chunkPool of the new chunk.
    size_t addUninitializedChunkBuffer()
    {
        size_t val = chunkPoolSize.load();
        size_t ret = val;
        chunkPoolSize.store(val+1);
        return ret;
    }

    void launchThreads(jl::Camera* camera, World* world)
    {

        meshBuildingThreadRunning = true;
        chunkWorker = std::thread(&WorldRenderer::meshBuildCoroutine, this,
            camera, world);
        //chunkWorker.detach();
        rebuildQueue.shouldExit = false;
        rebuildThreadRunning = true;
        rebuildThread = std::thread(&WorldRenderer::rebuildThreadFunction,
            this,
            world
        );
        //rebuildThread.detach();
    }

    void stopThreads()
    {
        rebuildThreadRunning = false;
        meshBuildingThreadRunning = false;
        chunkWorker.join();
        rebuildQueue.signalExit();
        rebuildThread.join();
    }

    void rebuildThreadFunction(World* world);

    void requestBlockBulkPlaceFromMainThread(const BlockArea& area)
    {
        rebuildQueue.push(ChunkRebuildRequest(area
                ));
    }

    void requestVoxelModelPlaceFromMainThread(const PlacedVoxModel& vox)
    {
        rebuildQueue.push(ChunkRebuildRequest(vox));
    }

    void requestChunkRebuildFromMainThread(const IntTup& spot, std::optional<BlockType> changeTo = std::nullopt, bool rebuild = true, const glm::vec3& pp = glm::vec3(0.f), bool lightpass = false, bool queuelightimplic = true)
    {
        //TwoIntTup version of spot
        auto titspot = TwoIntTup(spot.x, spot.z);
        auto chunkspot = WorldRenderer::worldToChunkPos(titspot);
        if (activeChunks.contains(chunkspot))
        {
            //std::cout << "Requesting rebuild for spot: " << chunkspot.x << " " << chunkspot.z << std::endl;
            if(changeTo != std::nullopt)
            {
                auto req = ChunkRebuildRequest(
                    chunkspot,
                    activeChunks.at(chunkspot).chunkIndex,
                    true,
                    spot,
                    changeTo.value(),
                    rebuild,
                    pp
                );
                req.doLightPass = lightpass;
                req.queueLightpassImplicated = queuelightimplic;
                rebuildQueue.push(req);
            } else
            {
                auto req = ChunkRebuildRequest(
                    chunkspot,
                    activeChunks.at(chunkspot).chunkIndex,
                    true
                );
                req.doLightPass = lightpass;
                req.queueLightpassImplicated = queuelightimplic;
                rebuildQueue.push(req);
            }

        } else
        {

        }
    }

    void printMemoryFootprint() const {
    size_t total = 0;

    std::cout << "Memory Footprint of WorldRenderer (in bytes):\n";
    std::cout << "---------------------------------------------\n";

    // Static and primitive members
    auto add_size = [&](const char* name, size_t size) {
        std::cout << name << ": " << size << " bytes\n";
        total += size;
    };

    add_size("currentRenderDistance (int)", sizeof(currentRenderDistance));
    add_size("MIN_DISTANCE (int)", sizeof(MIN_DISTANCE));
    add_size("lastMaxChunks (int)", sizeof(lastMaxChunks));
    add_size("chunkPoolSize (std::atomic<size_t>)", sizeof(chunkPoolSize));
    add_size("rebuildThreadRunning (std::atomic<int>)", sizeof(rebuildThreadRunning));
    add_size("meshBuildingThreadRunning (std::atomic<int>)", sizeof(meshBuildingThreadRunning));

    // chunkPool (std::array<SmallChunkGLInfo, maxChunks>)
    size_t chunkPoolSize = sizeof(chunkPool);
    add_size("chunkPool (std::array<SmallChunkGLInfo, maxChunks>)", chunkPoolSize);

    // rebuildQueue (assuming it's a custom queue, sizeof gives static size)
    add_size("rebuildQueue (RebuildQueue)", sizeof(rebuildQueue));

    // Threads (sizeof gives static size, stack size not included)
    add_size("rebuildThread (std::thread)", sizeof(rebuildThread));
    add_size("chunkWorker (std::thread)", sizeof(chunkWorker));

    // Synchronization primitives
    add_size("bufferMutex (std::mutex)", sizeof(bufferMutex));
    add_size("bufferCV (std::condition_variable)", sizeof(bufferCV));
    add_size("mbtBufferMutex (std::mutex)", sizeof(mbtBufferMutex));
    add_size("mbtBufferCV (std::condition_variable)", sizeof(mbtBufferCV));

    // activeChunks (boost::unordered_flat_map)
    size_t activeChunksSize = sizeof(activeChunks) + (sizeof(std::pair<TwoIntTup, ReadyToDrawChunkInfo>) * activeChunks.bucket_count());
    add_size("activeChunks (boost::unordered_flat_map)", activeChunksSize);

    // mbtActiveChunks (boost::unordered_flat_map)
    size_t mbtActiveChunksSize = sizeof(mbtActiveChunks) + (sizeof(std::pair<TwoIntTup, UsedChunkInfo>) * mbtActiveChunks.bucket_count());
    add_size("mbtActiveChunks (boost::unordered_flat_map)", mbtActiveChunksSize);

    // generatedChunks (boost::unordered_flat_set)
    size_t generatedChunksSize = sizeof(generatedChunks) + (sizeof(TwoIntTup) * generatedChunks.bucket_count());
    add_size("generatedChunks (boost::unordered_flat_set)", generatedChunksSize);

    // changeBuffers (std::array<ChangeBuffer, 128>)
    size_t changeBuffersSize = sizeof(changeBuffers);
    add_size("changeBuffers (std::array<ChangeBuffer, 128>)", changeBuffersSize);

    // freedChangeBuffers (boost::lockfree::spsc_queue)
    size_t freedChangeBuffersSize = sizeof(freedChangeBuffers) + (sizeof(size_t) * 128); // Capacity is 128
    add_size("freedChangeBuffers (boost::lockfree::spsc_queue<size_t, 128>)", freedChangeBuffersSize);

    // userChangeMeshBuffers (std::array<ChangeBuffer, 128>)
    size_t userChangeMeshBuffersSize = sizeof(userChangeMeshBuffers);
    add_size("userChangeMeshBuffers (std::array<ChangeBuffer, 128>)", userChangeMeshBuffersSize);

    // freedUserChangeMeshBuffers (boost::lockfree::spsc_queue)
    size_t freedUserChangeMeshBuffersSize = sizeof(freedUserChangeMeshBuffers) + (sizeof(size_t) * 128); // Capacity is 128
    add_size("freedUserChangeMeshBuffers (boost::lockfree::spsc_queue<size_t, 128>)", freedUserChangeMeshBuffersSize);

    // confirmedActiveChunksQueue (boost::lockfree::spsc_queue)
    size_t confirmedActiveChunksQueueSize = sizeof(confirmedActiveChunksQueue) + (sizeof(TwoIntTup) * 128); // Capacity is 128
    add_size("confirmedActiveChunksQueue (boost::lockfree::spsc_queue<TwoIntTup, 128>)", confirmedActiveChunksQueueSize);

    // rebuildToMainAreaNotifications (boost::lockfree::spsc_queue)
    size_t rebuildToMainAreaNotificationsSize = sizeof(rebuildToMainAreaNotifications) + (sizeof(BlockArea) * 256); // Capacity is 256
    add_size("rebuildToMainAreaNotifications (boost::lockfree::spsc_queue<BlockArea, 256>)", rebuildToMainAreaNotificationsSize);

    std::cout << "---------------------------------------------\n";
    std::cout << "Total estimated memory footprint: " << total << " bytes (" << total / 1024.0 << " KB)\n";
    std::cout << "Note: Sizes for containers (e.g., unordered_map, spsc_queue) include allocated capacity. "
              << "Thread stack sizes and dynamic allocations are not included.\n";
}

    void requestChunkSpotRebuildFromMainThread(const TwoIntTup& chunkspot, bool priority = true)
    {
        if (activeChunks.contains(chunkspot))
        {
            //std::cout << "Requesting rebuild for spot: " << chunkspot.x << " " << chunkspot.z << std::endl;

            rebuildQueue.push(ChunkRebuildRequest(
                chunkspot,
                activeChunks.at(chunkspot).chunkIndex,
                priority
            ));


        } else
        {

        }
    }

    static TwoIntTup worldToChunkPos(const TwoIntTup& worldPos)
    {
        return {
            static_cast<int>(std::floor(static_cast<float>(worldPos.x) / chunkSize)),
            static_cast<int>(std::floor(static_cast<float>(worldPos.z) / chunkSize))
        };
    }

};



#endif //WORLDRENDERER_H
