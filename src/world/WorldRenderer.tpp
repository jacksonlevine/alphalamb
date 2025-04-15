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
UsableMesh fromChunk(const TwoIntTup& spot, World* world, int chunkSize, bool locked, bool light)
{
#ifdef MEASURE_CHUNKREB
    static size_t lookup_count = 0;
    static std::chrono::duration<double> cumulative_lookup_time = std::chrono::duration<double>::zero();
#endif

#ifdef MEASURE_CHUNKREB
    auto startt = std::chrono::high_resolution_clock::now();
#endif

    if (light || !litChunks.contains(spot))
    {
        std::cout << "Doing it with light " << std::endl;
        lightPassOnChunk<queueimplics>(world, spot, chunkSize, 250, locked);
        litChunks.insert(spot);
    }


    UsableMesh mesh;
    PxU32 index = 0;
    PxU32 tindex = 0;

    IntTup start(spot.x * chunkSize, spot.z * chunkSize);

    // Pre-cache chunk data to avoid repeated world lookups
    std::vector<BlockType> chunkData(chunkSize * chunkSize * 250, AIR);

    // Pre-compute whether each block is transparent
    std::vector<bool> isTransparent(chunkSize * chunkSize * 250, true);

    // Fill cache
    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            for (int y = 0; y < 250; y++) {
                IntTup here = start + IntTup(x, y, z);
                int idx = (x * chunkSize * 250) + (z * 250) + y;

                chunkData[idx] = locked ? world->getRawLocked(here) : world->getRaw(here);
                isTransparent[idx] = (chunkData[idx] == AIR) ||
                    (std::ranges::find(transparents, (chunkData[idx] & BLOCK_ID_BITS)) != transparents.end());
            }
        }
    }

    // Helper to get block from cache with bounds checking
    auto getBlock = [&](int x, int y, int z) -> BlockType {
        // Out of bounds check
        if (x < 0 || x >= chunkSize || y < 0 || y >= 250 || z < 0 || z >= chunkSize) {
            IntTup pos = start + IntTup(x, y, z);
            return locked ? world->getRawLocked(pos) : world->getRaw(pos);
        }

        int idx = (x * chunkSize * 250) + (z * 250) + y;
        return chunkData[idx];
    };

    // Helper to check transparency
    auto isBlockTransparent = [&](int x, int y, int z) -> bool {
        // Out of bounds check
        if (x < 0 || x >= chunkSize || y < 0 || y >= 250 || z < 0 || z >= chunkSize) {
            IntTup pos = start + IntTup(x, y, z);
            BlockType block = locked ? world->getRawLocked(pos) : world->getRaw(pos);
            return (block == AIR) || (std::ranges::find(transparents, block & BLOCK_ID_BITS) != transparents.end());
        }

        int idx = (x * chunkSize * 250) + (z * 250) + y;
        return isTransparent[idx];
    };

    // Process the chunk
    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            for (int y = 0; y < 250; y++) {
                int idx = (x * chunkSize * 250) + (z * 250) + y;
                BlockType rawBlockHere = chunkData[idx];
                uint32_t blockID = (rawBlockHere & BLOCK_ID_BITS);
                MaterialName mat = static_cast<MaterialName>(blockID);

                // Skip air blocks
                if (blockID == AIR) continue;

                IntTup here = start + IntTup(x, y, z);

                if (auto specialFunc = findSpecialBlockMeshFunc(mat); specialFunc != std::nullopt)
                {
                    specialFunc.value()(mesh, rawBlockHere, here, index, tindex);
                } else
                {
                    bool blockHereTransparent = isTransparent[idx];


                    // Check each neighbor direction
                    for (int i = 0; i < std::size(neighborSpots); i++) {
                        auto neigh = neighborSpots[i];
                        int nx = x + neigh.x;
                        int ny = y + neigh.y;
                        int nz = z + neigh.z;

                        BlockType neighblock = (getBlock(nx, ny, nz) & BLOCK_ID_BITS);
                        bool neightransparent = isBlockTransparent(nx, ny, nz);
                        bool neighborair = neighblock == AIR;
                        bool solidNeighboringTransparent = (neightransparent && !blockHereTransparent);

                        if (neighborair || solidNeighboringTransparent || (blockHereTransparent && (neighblock != blockID) && neightransparent)) {
                            Side side = static_cast<Side>(i);

                            uint16_t blockbright = 0;
                            uint16_t ambientbright = 0;
                            auto ns = here + neighborSpots[(int)side];
                            if (locked)
                            {
                                if (lightmap.contains(ns))
                                {
                                    blockbright = lightmap.at(ns).sum();
                                }
                                if (ambientlightmap.contains(ns))
                                {
                                    ambientbright = ambientlightmap.at(ns).sum();
                                }
                            } else
                            {
                                auto lock =std::shared_lock<std::shared_mutex>(lightmapMutex);
                                if (lightmap.contains(ns))
                                {
                                    blockbright = lightmap.at(ns).sum();
                                }
                                if (ambientlightmap.contains(ns))
                                {
                                    ambientbright = ambientlightmap.at(ns).sum();
                                }
                            }

                            auto blockandambbright = getBlockAmbientLightVal(blockbright, ambientbright);


                            if (blockID == WATER && side == Side::Top)
                            {
                                if (!ambOccl) {
                                    addFace<true>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                                 side, static_cast<MaterialName>(blockID), 1, mesh, index, tindex, -0.2f);
                                    addFace<true>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y)+1, static_cast<float>(here.z)),
                                                Side::Bottom, static_cast<MaterialName>(blockID), 1, mesh, index, tindex, -0.2f);
                                } else {
                                    addFace<false>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                                  side, static_cast<MaterialName>(blockID), 1, mesh, index, tindex, -0.2f);

                                    calculateAmbientOcclusion(here, side, world, locked, blockID, mesh, blockandambbright);
                                    addFace<false>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y)+1, static_cast<float>(here.z)),
                                                  Side::Bottom, static_cast<MaterialName>(blockID), 1, mesh, index, tindex, -0.2f);
                                    calculateAmbientOcclusion(here, side, world, locked, blockID, mesh, blockandambbright);
                                }

                            } else
                            {
                                if (!ambOccl) {
                                    addFace<true>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                                 side, static_cast<MaterialName>(blockID), 1, mesh, index, tindex);
                                } else {
                                    addFace<false>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                                  side, static_cast<MaterialName>(blockID), 1, mesh, index, tindex);
                                    calculateAmbientOcclusion(here, side, world, locked, blockID, mesh, blockandambbright);
                                }
                            }
                        }
                    }
                }


            }
        }
    }

#ifdef MEASURE_CHUNKREB
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - startt;

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
__inline void addFace(PxVec3 offset, Side side, MaterialName material, int sideHeight, UsableMesh& mesh, PxU32& index, PxU32& tindex, float offsety)
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
    if (std::ranges::find(transparents, material) != transparents.end())
    {
        std::ranges::transform(cubefaces[static_cast<int>(side)], std::back_inserter(mesh.tpositions), [offset, sideHeight, offsety](const auto& v) {
            auto newv = v;
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
            float isGrass = material == GRASS ? 1.0f : 0.0f;

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
            float isGrass = material == GRASS ? 1.0f : 0.0f;

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
