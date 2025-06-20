#include "WorldRenderer.h"

#include "MaterialName.h"
#include "../AmbOcclSetting.h"
#include "worldgenmethods/OverworldWorldGenMethod.h"
#include "../IndexOptimization.h"
#include "../specialblocks/FindEntityCreateFunc.h"
#include "../specialblocks/FindSpecialBlock.h"
#include "../Light.h"




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
        int idx = (x * chunkSize * chunkHeight) + (z * chunkHeight) + y;
        BlockType rawBlockHere = chunkData[idx];
        uint32_t blockID = (rawBlockHere & BLOCK_ID_BITS);
        MaterialName mat = static_cast<MaterialName>(blockID);

        IntTup here = start + IntTup(x, y, z);

        if (auto specialFunc = findSpecialBlockMeshFunc(mat); specialFunc != std::nullopt) {
            specialFunc.value()(mesh, rawBlockHere, here, index, tindex);
        } else {
            bool blockHereTransparent = isTransparent[idx];

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
                        if (!ambOccl) {
                            addFace<true>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                         side, mat, 1, mesh, index, tindex, -0.2f);
                            addFace<true>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y) + 1, static_cast<float>(here.z)),
                                         Side::Bottom, mat, 1, mesh, index, tindex, -0.2f);
                        } else {
                            addFace<false>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                          side, mat, 1, mesh, index, tindex, -0.2f);
                            calculateAmbientOcclusion(here, side, world, locked, blockID, mesh, blockAndAmbBright);
                            addFace<false>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y) + 1, static_cast<float>(here.z)),
                                          Side::Bottom, mat, 1, mesh, index, tindex, -0.2f);
                            calculateAmbientOcclusion(here, side, world, locked, blockID, mesh, blockAndAmbBright);
                        }
                    } else {
                        if (!ambOccl) {
                            addFace<true>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                         side, mat, 1, mesh, index, tindex);
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
///Call this with an external index and UsableMesh to mutate them
template<bool doBrightness>
__inline void addFace(PxVec3 offset, Side side, MaterialName material, int sideHeight, UsableMesh& mesh, PxU32& index, PxU32& tindex, float offsety, float pushIn)
{
    auto & tex = TEXS[material];
    auto faceindex = (side == Side::Top) ? 2 : (side == Side::Bottom) ? 1 : 0;
    auto & face = tex[faceindex];

    float uvoffsetx = static_cast<float>(face.first) * texSlotWidth;
    float uvoffsety = 1.0f - (static_cast<float>(face.second) * texSlotWidth);

    static glm::vec2 texOffsets[] = {
        glm::vec2(onePixel, -onePixel),
        glm::vec2(onePixel + textureWidth, -onePixel),
        glm::vec2(onePixel + textureWidth, -(onePixel + textureWidth)),
        glm::vec2(onePixel, -(onePixel + textureWidth)),
    };
    //If the material's transparent, add these verts to the "t____" parts of the mesh. If not, add them to the normal mesh.
    if (transparents.test(material))
    {
        std::ranges::transform(cubefaces[static_cast<int>(side)], std::back_inserter(mesh.tpositions), [offset, sideHeight, offsety, pushIn](const auto& v) {
            auto newv = PxVec3(pushIn, pushIn, pushIn) + v*(1.f- pushIn);
            newv.y *= sideHeight;
            return newv + offset + PxVec3(0.f, offsety, 0.f);
        });

        std::ranges::transform(cwindices, std::back_inserter(mesh.tindices), [&tindex](const auto& i) {
            return tindex + i;
        });

        mesh.ttexcoords.insert(mesh.ttexcoords.end(),{
                glm::vec2(uvoffsetx + texOffsets[0].x, uvoffsety + texOffsets[0].y),
            glm::vec2(uvoffsetx + texOffsets[1].x, uvoffsety + texOffsets[1].y),
            glm::vec2(uvoffsetx + texOffsets[2].x, uvoffsety + texOffsets[2].y),
            glm::vec2(uvoffsetx + texOffsets[3].x, uvoffsety + texOffsets[3].y),
            });

        if constexpr(doBrightness)
        {
            float isGrass = grasstypes.test(material) ? 1.0f : 0.0f;

            switch(side) {
            case Side::Top:    mesh.tbrightness.insert(mesh.tbrightness.end(), {1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass}); break;
            case Side::Left:   mesh.tbrightness.insert(mesh.tbrightness.end(), {0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass}); break;
            case Side::Bottom: mesh.tbrightness.insert(mesh.tbrightness.end(), {0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass}); break;
            case Side::Right:  mesh.tbrightness.insert(mesh.tbrightness.end(), {0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass}); break;
            default:          mesh.tbrightness.insert(mesh.tbrightness.end(), {0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass});
            }
        }


        tindex += 4;
    } else
    {
        std::ranges::transform(cubefaces[static_cast<int>(side)], std::back_inserter(mesh.positions), [offset, sideHeight, offsety](const auto& v) {
            auto newv = v;
            newv.y *= sideHeight;
            return newv + offset + PxVec3(0.f, offsety, 0.f);
        });

        std::ranges::transform(cwindices, std::back_inserter(mesh.indices), [&index](const auto& i) {
            return index + i;
        });

        mesh.texcoords.insert(mesh.texcoords.end(),
{glm::vec2(uvoffsetx + texOffsets[0].x, uvoffsety + texOffsets[0].y),
glm::vec2(uvoffsetx + texOffsets[1].x, uvoffsety + texOffsets[1].y),
glm::vec2(uvoffsetx + texOffsets[2].x, uvoffsety + texOffsets[2].y),
glm::vec2(uvoffsetx + texOffsets[3].x, uvoffsety + texOffsets[3].y),});

        if constexpr(doBrightness)
        {
            float isGrass = grasstypes.test(material) ? 1.0f : 0.0f;

            switch(side) {
            case Side::Top:    mesh.brightness.insert(mesh.brightness.end(), {1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass}); break;
            case Side::Left:   mesh.brightness.insert(mesh.brightness.end(), {0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass}); break;
            case Side::Bottom: mesh.brightness.insert(mesh.brightness.end(), {0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass}); break;
            case Side::Right:  mesh.brightness.insert(mesh.brightness.end(), {0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass}); break;
            default:          mesh.brightness.insert(mesh.brightness.end(), {0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass});
            }
        }
        index += 4;
    }



}
