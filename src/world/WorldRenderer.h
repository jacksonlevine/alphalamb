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

__inline void addFace(PxVec3 offset, Side side, MaterialName material, int sideHeight, UsableMesh& mesh, uint32_t& index, uint32_t& tindex);
void genCGLBuffers();
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


UsableMesh fromChunk(const TwoIntTup& spot, World* world, int chunkSize);
UsableMesh fromChunkLocked(const TwoIntTup& spot, World* world, int chunkSize);

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


class WorldRenderer {
public:
    static constexpr int chunkSize = 16;
    static constexpr int renderDistance = 18;
    static constexpr int maxChunks = ((renderDistance*2) * (renderDistance*2));
    static constexpr int MIN_DISTANCE = renderDistance + 1;

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
    void generateChunk(World* world, const TwoIntTup& chunkSpot, std::unordered_set<TwoIntTup, TwoIntTupHash>& implicatedChunks);

    WorldRenderer()
    {
        mbtActiveChunks.reserve(maxChunks);
        activeChunks.reserve(maxChunks);
        //Generate them all at the beginning so we know the buffer names for sure
        std::ranges::for_each(std::views::iota(0, maxChunks), [](auto) {
            genCGLBuffers();
        });
    }



    ///A limited list of atomic "Change Buffers" that the mesh building thread can reserve and write to, and the main thread will "check its mail", do the necessary GL calls, and re-free the Change Buffers
    ///by adding its index to freeChangeBuffers.
    std::array<ChangeBuffer, 10> changeBuffers = {};
    ///One way queue, from main thread to mesh building thread, to notify of freed Change Buffers
    boost::lockfree::spsc_queue<size_t, boost::lockfree::capacity<10>> freedChangeBuffers = {};


    ///A limited list of atomic "Change Buffers" that the mesh building thread can reserve and write to, and the main thread will "check its mail", do the necessary GL calls, and re-free the Change Buffers
    ///by adding its index to freeChangeBuffers.
    std::array<ChangeBuffer, 30> userChangeMeshBuffers = {};
    ///One way queue, from main thread to mesh building thread, to notify of freed Change Buffers
    boost::lockfree::spsc_queue<size_t, boost::lockfree::capacity<128>> freedUserChangeMeshBuffers = {};

    ///After being added to mbtActiveChunks, we await a confirmation back in this before we know we can reuse that chunk again
    ///One way queue, from main thread to mesh building thread, to notify of mbtActiveChunks entries that have been confirmed/entered into activeChunks.
    boost::lockfree::spsc_queue<TwoIntTup, boost::lockfree::capacity<128>> confirmedActiveChunksQueue = {};



    //A way for the rebuild thread to ask the main thread "What chunks are active from this area?" so that the main thread can submit rebuild requests for them
    boost::lockfree::spsc_queue<BlockArea, boost::lockfree::capacity<256>> rebuildToMainAreaNotifications = {};



    void mainThreadDraw(const jl::Camera* playerCamera, GLuint shader, WorldGenMethod* worldGenMethod, float deltaTime);
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

    void rebuildThreadFunction(World* world) {
        std::cout << "Rebuild thread started!\n";
        NUM_THREADS_RUNNING.fetch_add(1);  // Atomic increment
        //std::cout << "Rebuild thread incremented NUM_THREADS_RUNNING. Current value: " << NUM_THREADS_RUNNING.load() << "\n";

        while (rebuildThreadRunning) {
            ChunkRebuildRequest request;
            //std::cout << "Running \n";
            if (rebuildQueue.pop(request)) {
                //std::cout << "Popped one: " << request.chunkPos.x << " " << request.chunkPos.z << " \n";

                if (request.isArea)
                {
                    std::vector<IntTup> spotsToEraseInUDM;
                    spotsToEraseInUDM.reserve(500);


                    {
                        //We're gonna do the block placing, then ask main to request the rebuilds because we won't know what chunks are active when we're done.
                        std::unordered_set<TwoIntTup, TwoIntTupHash> implicatedChunks;
                        auto lock = world->nonUserDataMap->getUniqueLock();
                        std::shared_lock<std::shared_mutex> udmRL(world->userDataMap->mutex());
                        auto m = request.area;
                        int minX = std::min(m.corner1.x, m.corner2.x);
                        int maxX = std::max(m.corner1.x, m.corner2.x);
                        int minY = std::min(m.corner1.y, m.corner2.y);
                        int maxY = std::max(m.corner1.y, m.corner2.y);
                        int minZ = std::min(m.corner1.z, m.corner2.z);
                        int maxZ = std::max(m.corner1.z, m.corner2.z);

                        for (int x = minX; x <= maxX; x++) {
                            for (int y = minY; y <= maxY; y++) {
                                for (int z = minZ; z <= maxZ; z++) {
                                    bool isBoundary = (x == minX || x == maxX ||
                                                       y == minY || y == maxY ||
                                                       z == minZ || z == maxZ);

                                    if (isBoundary || !m.hollow) {
                                        world->nonUserDataMap->setLocked(IntTup{x, y, z}, m.block);
                                        if (world->userDataMap->getLocked(IntTup{x, y, z}) != std::nullopt)
                                        {
                                            spotsToEraseInUDM.emplace_back(x, y, z);
                                        }
                                    }

                                }
                            }
                        }
                    }

                    {
                        auto lock = world->userDataMap->getUniqueLock();
                        for (auto & spot : spotsToEraseInUDM)
                        {
                            world->userDataMap->erase(spot, true);
                        }
                    }

                    if (rebuildToMainAreaNotifications.write_available() > 0)
                    {
                        rebuildToMainAreaNotifications.push(request.area);
                    } else
                    {
                        std::cout << "No more space in rebuildtomainareanotifications \n";
                    }


                } else
                if (request.isVoxelModel)
                {
                    auto & vm = voxelModels[request.vm.name];
                    std::vector<IntTup> spotsToEraseInUDM;
                    spotsToEraseInUDM.reserve(500);

                    {
                        //We're gonna do the block placing, then ask main to request the rebuilds because we won't know what chunks are active when we're done.
                        std::unordered_set<TwoIntTup, TwoIntTupHash> implicatedChunks;
                        auto lock = world->nonUserDataMap->getUniqueLock();
                        std::shared_lock<std::shared_mutex> udmRL(world->userDataMap->mutex());

                        IntTup offset = IntTup(vm.dimensions.x/-2, 0, vm.dimensions.z/-2) + request.vm.spot;
                        for ( auto & p : vm.points)
                        {
                            world->nonUserDataMap->setLocked(offset+p.localSpot, p.colorIndex);
                            if (world->userDataMap->getLocked(offset+p.localSpot) != std::nullopt)
                            {
                                spotsToEraseInUDM.emplace_back(offset+p.localSpot);
                            }

                        }



                    }
                    {
                        auto lock = world->userDataMap->getUniqueLock();
                        for (auto & spot : spotsToEraseInUDM)
                        {
                            world->userDataMap->erase(spot, true);
                        }
                    }
                    if (rebuildToMainAreaNotifications.write_available() > 0)
                    {
                        IntTup corner1 = IntTup(vm.dimensions.x/-2, 0, vm.dimensions.z/-2) + request.vm.spot;
                        IntTup corner2 = IntTup(vm.dimensions.x/2, vm.dimensions.y, vm.dimensions.z/2) + request.vm.spot;

                        rebuildToMainAreaNotifications.push(BlockArea{
                            corner1, corner2, AIR, false
                        });
                    } else
                    {
                        std::cout << "No more space in rebuildtomainareanotifications \n";
                    }

                }else
                {
                    if(request.changeTo != std::nullopt)
                    {
                        //std::cout <<"Doing the fucking write to " << request.changeSpot.x << " " << request.changeSpot.y << " " << request.changeSpot.z << " \n";
                        world->set(request.changeSpot, request.changeTo.value());
                    }
                    if(request.rebuild)
                    {
                        UsableMesh mesh;
                        // Scope the locks so they're released after getting data
                        {
                            if (auto locks = world->tryToGetReadLockOnDMs()) {
                                // Get the chunk data with locks held
                                //std::cout << "Got readlock on dms \n";
                                mesh = fromChunkLocked(request.chunkPos, world, chunkSize);
                            } else if (rebuildThreadRunning) {
                                std::cout << "Failed to get read lock on DMs\n";
                                rebuildQueue.push(request);
                                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                continue;
                            }
                        } // locks are released here


                        // Wait for available buffer using condition variable
                        size_t changeBufferIndex = -1;
                        {
                            std::unique_lock<std::mutex> lock(bufferMutex);
                            bufferCV.wait(lock, [&]() {
                                return freedUserChangeMeshBuffers.pop(changeBufferIndex) || !rebuildThreadRunning;
                            });

                            if (!rebuildThreadRunning) break;
                        }

                        if (changeBufferIndex != -1)
                        {
                            auto& buffer = userChangeMeshBuffers[changeBufferIndex];
                            buffer.in_use.store(true);
                            buffer.mesh = mesh;
                            buffer.chunkIndex = request.chunkIndex;
                            buffer.from = request.chunkPos;
                            buffer.to = request.chunkPos;
                            buffer.ready.store(true);
                            buffer.in_use.store(false);
                        }
                    }
                }





            }
        }
        NUM_THREADS_RUNNING.fetch_sub(1);
        std::cout << "Rebuild thread finished!\n";
    }

    void requestBlockBulkPlaceFromMainThread(const BlockArea& area)
    {
        rebuildQueue.push(ChunkRebuildRequest(area
                ));
    }

    void requestVoxelModelPlaceFromMainThread(const PlacedVoxModel& vox)
    {
        rebuildQueue.push(ChunkRebuildRequest(vox));
    }

    void requestChunkRebuildFromMainThread(const IntTup& spot, std::optional<uint32_t> changeTo = std::nullopt, bool rebuild = true)
    {
        //TwoIntTup version of spot
        auto titspot = TwoIntTup(spot.x, spot.z);
        auto chunkspot = WorldRenderer::worldToChunkPos(titspot);
        if (activeChunks.contains(chunkspot))
        {
            //std::cout << "Requesting rebuild for spot: " << chunkspot.x << " " << chunkspot.z << std::endl;
            if(changeTo != std::nullopt)
            {
                rebuildQueue.push(ChunkRebuildRequest(
                    chunkspot,
                    activeChunks.at(chunkspot).chunkIndex,
                    true,
                    spot,
                    changeTo.value(),
                    rebuild
                ));
            } else
            {
                rebuildQueue.push(ChunkRebuildRequest(
                    chunkspot,
                    activeChunks.at(chunkspot).chunkIndex,
                    true
                ));
            }

        } else
        {
            std::cout << "Spot not in activeChunks \n";
        }
    }

    void requestChunkSpotRebuildFromMainThread(const TwoIntTup& chunkspot)
    {
        if (activeChunks.contains(chunkspot))
        {
            //std::cout << "Requesting rebuild for spot: " << chunkspot.x << " " << chunkspot.z << std::endl;

            rebuildQueue.push(ChunkRebuildRequest(
                chunkspot,
                activeChunks.at(chunkspot).chunkIndex,
                true
            ));


        } else
        {
            std::cout << "Spot not in activeChunks \n";
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
