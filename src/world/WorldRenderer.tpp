#include "WorldRenderer.h"

#include "MaterialName.h"
#include "../AmbOcclSetting.h"
#include "worldgenmethods/OverworldWorldGenMethod.h"
#include "../IndexOptimization.h"
#include "../specialblocks/FindEntityCreateFunc.h"
#include "../specialblocks/FindSpecialBlock.h"
#include "../Light.h"
#include "../MarchingCubesLookups.h"




///Create a UsableMesh from the specified chunk spot
///This gets called in the mesh building coroutine and rebuild coroutine
///Queueimplics = default true, whether to queue chunks implicated by light pass for rebuild

template<bool queueimplics>
UsableMesh fromChunk(const TwoIntTup& spot, World* world, bool locked, bool light)
{
#ifdef MEASURE_CHUNKREB
    auto startt = std::chrono::high_resolution_clock::now();
#endif

    if(locked)
    {
        std::cout << "Locked " << std::endl;
    }
    UsableMesh mesh;
    PxU32 index = 0;
    PxU32 tindex = 0;
    constexpr int chunkSize = 16;
    IntTup start(spot.x * chunkSize, spot.z * chunkSize);
    constexpr int chunkHeight = 250;

    // Cache chunk data and transparency
    std::vector<BlockType> chunkData(chunkSize * chunkSize * chunkHeight, AIR);
    std::vector<bool> isTransparent(chunkSize * chunkSize * chunkHeight, true);

    std::vector<bool> marchedOrigs(chunkSize * chunkSize * chunkHeight, false);


    // Light source collections (only used if light = true)
    std::vector<std::pair<IntTup, ColorPack>> oldBlockSources;
    std::vector<std::pair<IntTup, ColorPack>> newBlockSources;
    std::vector<std::pair<IntTup, ColorPack>> oldAmbientSources;
    std::vector<std::pair<IntTup, ColorPack>> newAmbientSources;

    // Track blocks to process for meshing
    std::vector<std::tuple<int, int, int>> blocksToMesh;

    std::unordered_set<TwoIntTup, TwoIntTupHash> implicatedChunks;

    // Check if lighting is needed
    bool doLight = light;
    if (!doLight) {
        tbb::concurrent_hash_map<TwoIntTup, bool, TwoIntTupHashCompare>::const_accessor acc;
        if (!litChunks.find(acc, spot)) {
            doLight = true;
        }
    }



    static std::array mcoversamplingspots = {
        IntTup(0,0,-1),
        IntTup(-1, 0, -1),
        IntTup(-1, 0, 0),
        IntTup(0,-1,-1),
        IntTup(-1, -1, -1),
        IntTup(-1, -1, 0),
        IntTup(0, -1, 0)
    };

    // SCollect block data, light sources (if doLight), and blocks to mesh
    if (!locked) {
        std::shared_lock<std::shared_mutex> url(world->userDataMap.mutex());
        std::shared_lock<std::shared_mutex> nrl(world->nonUserDataMap.mutex());
        std::shared_lock<std::shared_mutex> lightLock;
        if(doLight) {
            lightLock = std::shared_lock<std::shared_mutex>(lightmapMutex);
        }

        for (int x = 0; x < chunkSize; x++) {
            for (int z = 0; z < chunkSize; z++) {
                bool foundGround = false;
                bool hitSolid = false;
                for (int y = chunkHeight - 1; y >= 0; y--) {
                    IntTup here = start + IntTup(x, y, z);
                    int idx = (x * chunkSize * chunkHeight) + (z * chunkHeight) + y;

                    // Cache block data
                    chunkData[idx] = world->getRawLocked(here);
                    isTransparent[idx] = (chunkData[idx] == AIR) ||
                                        (transparents.test(chunkData[idx] & BLOCK_ID_BITS));

                    // Light source collection (if doLight)
                    if (doLight) {
                        BlockType h = chunkData[idx];
                        auto originhash = IntTupHash{}(here, true);
                        if (h == LIGHT) {
                            newBlockSources.emplace_back(here, TORCHLIGHTVAL);
                        }

                        // Check ambient lightmap
                        auto ambientSpot = ambientlightmap.get(here);
                        if (ambientSpot != std::nullopt) {
                            auto& val = ambientSpot.value();
                            for (int i = 0; i < val->count; i++) {
                                if (originhash == val->originhashes[i]) {
                                    oldAmbientSources.emplace_back(here, SKYLIGHTVAL);
                                }
                            }
                        }

                        // Check block lightmap
                        auto blockSpot = lightmap.get(here);
                        if (blockSpot != std::nullopt) {
                            auto& val = blockSpot.value();
                            for (int i = 0; i < val->count; i++) {
                                if (originhash == val->originhashes[i]) {
                                    oldBlockSources.emplace_back(here, TORCHLIGHTVAL);
                                }
                            }
                        }

                        // Ambient light source detection
                        if (h != AIR) {
                            if (!foundGround) {
                                newAmbientSources.push_back(std::make_pair(here + IntTup(0, 1, 0), SKYLIGHTVAL));
                                foundGround = true;
                            }
                            hitSolid = true;
                        } else if (!hitSolid) {
                            for (const auto& neighb : neighbs4) {
                                IntTup neighbPos = here + neighb;
                                // Use cache if neighbor is in-bounds
                                int nx = x + neighb.x;
                                int nz = z + neighb.z;
                                int ny = y + neighb.y;
                                BlockType neighBlock;
                                if (nx >= 0 && nx < chunkSize && ny >= 0 && ny < chunkHeight && nz >= 0 && nz < chunkSize) {
                                    int nidx = (nx * chunkSize * chunkHeight) + (nz * chunkHeight) + ny;
                                    neighBlock = chunkData[nidx];
                                } else {
                                    neighBlock = world->getLocked(neighbPos);
                                }
                                if (neighBlock != AIR) {
                                    newAmbientSources.push_back(std::make_pair(here, SKYLIGHTVAL));
                                    break;
                                }
                            }
                        }
                    }

                    // Mark non-air blocks for meshing
                    if ((chunkData[idx] & BLOCK_ID_BITS) != AIR) {
                        blocksToMesh.emplace_back(x, y, z);
                       // std::cout << "Pushed " << x << " " << y << " " << z << std::endl;
                        if (marchers.test(static_cast<MaterialName>(chunkData[idx] & BLOCK_ID_BITS)) && !marchedOrigs[idx])
                        {
                            marchedOrigs[idx] = true;
                           // std::cout << "Marchers test passed" << std::endl;



                            //blocksToMesh.emplace_back(x, y+1, z);
                            for (auto e : mcoversamplingspots)
                            {
                               // std::cout << "Pushed additional from thesespots: " << x+e.x << " " << y+e.y << " " << z+e.z << std::endl;
                                blocksToMesh.emplace_back(x+e.x, y+e.y, z+e.z);
                            }
                            // for (auto e : neighborSpots)
                            // {
                            //     blocksToMesh.emplace_back(x+e.x, y+e.y, z+e.z);
                            // }
                        }

                    }
                }
            }
        }
        // Locks released here (url, nrl, lightLock)
    } else {
        // Locked mode: No locks needed, access world directly
        for (int x = 0; x < chunkSize; x++) {
            for (int z = 0; z < chunkSize; z++) {
                bool foundGround = false;
                bool hitSolid = false;
                for (int y = chunkHeight - 1; y >= 0; y--) {
                    IntTup here = start + IntTup(x, y, z);
                    int idx = (x * chunkSize * chunkHeight) + (z * chunkHeight) + y;

                    // Cache block data
                    chunkData[idx] = world->getRawLocked(here);
                    isTransparent[idx] = (chunkData[idx] == AIR) ||
                                        (transparents.test(chunkData[idx] & BLOCK_ID_BITS));

                    // Light source collection (if doLight)
                    if (doLight) {
                        BlockType h = chunkData[idx];
                        auto originhash = IntTupHash{}(here, true);
                        if (h == LIGHT) {
                            newBlockSources.push_back(std::make_pair(here, TORCHLIGHTVAL));
                        }

                        // Check ambient lightmap
                        auto ambientSpot = ambientlightmap.get(here);
                        if (ambientSpot != std::nullopt) {
                            auto& val = ambientSpot.value();
                            for (int i = 0; i < val->count; i++) {
                                if (originhash == val->originhashes[i]) {
                                    oldAmbientSources.emplace_back(here, SKYLIGHTVAL);
                                }
                            }
                        }

                        // Check block lightmap
                        auto blockSpot = lightmap.get(here);
                        if (blockSpot != std::nullopt) {
                            auto& val = blockSpot.value();
                            for (int i = 0; i < val->count; i++) {
                                if (originhash == val->originhashes[i]) {
                                    oldBlockSources.emplace_back(here, TORCHLIGHTVAL);
                                }
                            }
                        }
                        // Ambient light source detection
                        if (h != AIR) {
                            if (!foundGround) {
                                newAmbientSources.push_back(std::make_pair(here + IntTup(0, 1, 0), SKYLIGHTVAL));
                                foundGround = true;
                            }
                            hitSolid = true;
                        } else if (!hitSolid) {
                            for (const auto& neighb : neighbs4) {
                                IntTup neighbPos = here + neighb;
                                // Use cache if neighbor is in-bounds
                                int nx = x + neighb.x;
                                int nz = z + neighb.z;
                                int ny = y + neighb.y;
                                BlockType neighBlock;
                                if (nx >= 0 && nx < chunkSize && ny >= 0 && ny < chunkHeight && nz >= 0 && nz < chunkSize) {
                                    int nidx = (nx * chunkSize * chunkHeight) + (nz * chunkHeight) + ny;
                                    neighBlock = chunkData[nidx];
                                } else {
                                    neighBlock = world->getLocked(neighbPos);
                                }
                                if (neighBlock != AIR) {
                                    newAmbientSources.push_back(std::make_pair(here, SKYLIGHTVAL));
                                    break;
                                }
                            }
                        }
                    }

                    // Mark non-air blocks for meshing
                    if ((chunkData[idx] & BLOCK_ID_BITS) != AIR) {
                        blocksToMesh.emplace_back(x, y, z);
                       // std::cout << "Pushed " << x << " " << y << " " << z << std::endl;
                        if (marchers.test(static_cast<MaterialName>(chunkData[idx] & BLOCK_ID_BITS)))
                        {

                         //   std::cout << "Marchers test passed" << std::endl;



                            //blocksToMesh.emplace_back(x, y+1, z);
                            for (auto e : mcoversamplingspots)
                            {
                               // std::cout << "Pushed additional from thesespots: " << x+e.x << " " << y+e.y << " " << z+e.z << std::endl;
                                blocksToMesh.emplace_back(x+e.x, y+e.y, z+e.z);
                            }
                            // for (auto e : neighborSpots)
                            // {
                            //     blocksToMesh.emplace_back(x+e.x, y+e.y, z+e.z);
                            // }
                        }

                    }
                }
            }
        }
        // lightLock released here (if doLight)
    }

    // Perform light propagation before meshing (if doLight)
    if (doLight) {
        if(!locked)
        {
            std::shared_lock<std::shared_mutex> url(world->userDataMap.mutex());
            std::shared_lock<std::shared_mutex> nrl(world->nonUserDataMap.mutex());
            auto lightlock = std::unique_lock<std::shared_mutex>(lightmapMutex);

            unpropagateAllLightsLayered(oldBlockSources, lightmap, spot, &implicatedChunks, true);
            propagateAllLightsLayered(world, newBlockSources, lightmap, spot, &implicatedChunks, true);
            unpropagateAllLightsLayered(oldAmbientSources, ambientlightmap, spot, &implicatedChunks, true);
            propagateAllLightsLayered(world, newAmbientSources, ambientlightmap, spot, &implicatedChunks, true);
        } else
        {
            unpropagateAllLightsLayered(oldBlockSources, lightmap, spot, &implicatedChunks, false);
            propagateAllLightsLayered(world, newBlockSources, lightmap, spot, &implicatedChunks, false);
            unpropagateAllLightsLayered(oldAmbientSources, ambientlightmap, spot, &implicatedChunks, false);
            propagateAllLightsLayered(world, newAmbientSources, ambientlightmap, spot, &implicatedChunks, false);
        }


        litChunks.insert({spot, true});
    }

    auto getBlock = [&start,locked,&world, &chunkData](int x, int y, int z) -> BlockType {
        if (x < 0 || x >= chunkSize || y < 0 || y >= chunkHeight || z < 0 || z >= chunkSize) {
            IntTup pos = start + IntTup(x, y, z);
            return !locked ? world->getRaw(pos) : world->getRawLocked(pos);
        }
        int idx = (x * chunkSize * chunkHeight) + (z * chunkHeight) + y;
        return chunkData[idx];
    };

    auto isBlockTransparent = [&start, locked, &world, &isTransparent](int x, int y, int z) -> bool {
        if (x < 0 || x >= chunkSize || y < 0 || y >= chunkHeight || z < 0 || z >= chunkSize) {
            IntTup pos = start + IntTup(x, y, z);
            BlockType block = !locked ? world->getRaw(pos) : world->getRawLocked(pos);
            return (block == AIR) || (transparents.test(block & BLOCK_ID_BITS));
        }
        int idx = (x * chunkSize * chunkHeight) + (z * chunkHeight) + y;
        return isTransparent[idx];
    };

    for (const auto& [x, y, z] : blocksToMesh) {
        //int idx = (x * chunkSize * chunkHeight) + (z * chunkHeight) + y;
        BlockType rawBlockHere = getBlock(x,y,z);
        uint32_t blockID = (rawBlockHere & BLOCK_ID_BITS);
        MaterialName mat = static_cast<MaterialName>(blockID);

        IntTup here = start + IntTup(x, y, z);

       // std::cout << "Processing " << here.x << " " << here.y << " " << here.z << std::endl;

            uint8_t configindex = 0;

            bool hasaircorns = false;
            bool hassolidcorns = false;

            MaterialName marchermathit = STONE;

            for (int i = 0; i < std::size(cornerPositions); i++) {
                auto neigh = cornerPositions[i];
                int nx = x + neigh.x;
                int ny = y + neigh.y;
                int nz = z + neigh.z;

                BlockType neighBlock = (getBlock(nx, ny, nz) & BLOCK_ID_BITS);

                if (transparents.test(neighBlock))
                {
                    hasaircorns = true;
                } else
                {
                    if (marchers.test(neighBlock))
                    {
                        hassolidcorns = true;
                        configindex |= (1 << i);
                        marchermathit = (MaterialName)neighBlock;
                    }
                }
            }

            if (hasaircorns && hassolidcorns)
            {
                addMarcher<true>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                              configindex, marchermathit, 1, mesh, index, tindex);
            }



              //  std::cout << "Hasaircorns: " << hasaircorns << " hassolidcorns: " << hassolidcorns << std::endl;



        if (!marchers.test(mat) && mat != AIR)
        {
            {
                if (auto specialFunc = findSpecialBlockMeshFunc(mat); specialFunc != std::nullopt) {
                    specialFunc.value()(mesh, rawBlockHere, here, index, tindex);
                } else {
                    bool blockHereTransparent = isBlockTransparent(x,y,z);
                    //
                    // for (int i = 0; i < std::size(cornerPositions); i++)
                    // {
                    //
                    // }
                    for (int i = 0; i < std::size(neighborSpots); i++) {
                        auto neigh = neighborSpots[i];
                        int nx = x + neigh.x;
                        int ny = y + neigh.y;
                        int nz = z + neigh.z;

                        BlockType neighBlock = (getBlock(nx, ny, nz) & BLOCK_ID_BITS);
                        bool neighTransparent = isBlockTransparent(nx, ny, nz);
                        bool neighborAir = neighBlock == AIR;
                        bool solidNeighboringTransparent = (neighTransparent && !blockHereTransparent);

                        if (neighborAir || solidNeighboringTransparent || (blockHereTransparent && (neighBlock != blockID) && neighTransparent)) {
                            Side side = static_cast<Side>(i);
                            IntTup ns = here + neighborSpots[(int)side];

                            ColorPack blockBright = {};
                            ColorPack ambientBright = {};
                            {
                                std::shared_lock<std::shared_mutex> lightLock;
                                if(!locked) {
                                    lightLock = std::shared_lock<std::shared_mutex>(lightmapMutex);
                                }
                                if (lightmap.get(ns) != std::nullopt) {
                                    blockBright = lightmap.get(ns).value()->sum();
                                }
                                if (ambientlightmap.get(ns) != std::nullopt) {
                                    ambientBright = ambientlightmap.get(ns).value()->sum();
                                }
                            }
                            auto blockAndAmbBright = getBlockAmbientLightVal(blockBright, ambientBright);

                            if (blockID == WATER && side == Side::Top) {

                                addFace<false>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                              side, mat, 1, mesh, index, tindex, -0.2f);
                                calculateAmbientOcclusion(here, side, world, locked, blockID, mesh, blockAndAmbBright);
                                addFace<false>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y) + 1, static_cast<float>(here.z)),
                                              Side::Bottom, mat, 1, mesh, index, tindex, -0.2f);
                                calculateAmbientOcclusion(here, side, world, locked, blockID, mesh, blockAndAmbBright);

                            } else {

                                addFace<false>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                              side, mat, 1, mesh, index, tindex);
                                calculateAmbientOcclusion(here, side, world, locked, blockID, mesh, blockAndAmbBright);

                            }
                        }
                    }
                }
            }
        }


    }

    // Queue implicated chunks (if lighting was performed)
    if (doLight && queueimplics) {
        for (const auto& spot2 : implicatedChunks) {
            //only push if its not already queued! (this may save lots of work)
            auto acc = tbb::concurrent_hash_map<TwoIntTup, bool, TwoIntTupHashCompare>::const_accessor();

            if(!lightOverlapsQueued.find(acc, spot2))
            {
                lightOverlapsQueued.insert({spot2, true});
                lightOverlapNotificationQueue.push(spot2);
            }
            
        }
    }

#ifdef MEASURE_CHUNKREB
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - startt;
    static size_t lookup_count = 0;
    static std::chrono::duration<double> cumulative_lookup_time = std::chrono::duration<double>::zero();
    cumulative_lookup_time += elapsed;
    lookup_count++;
    if (lookup_count % 1000 == 0) {
        double average_lookup_time = cumulative_lookup_time.count() / lookup_count;
    }
#endif

    return mesh;
}


