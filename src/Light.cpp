//
// Created by jack on 4/12/2025.
//

#include "Light.h"

#include "world/WorldRenderer.h"
boost::sync_queue<TwoIntTup> lightOverlapNotificationQueue = {};
tbb::concurrent_hash_map<TwoIntTup, bool, TwoIntTupHashCompare> lightOverlapsQueued = {};


std::unique_ptr<std::vector<std::byte>> lmbuffer;
std::unique_ptr<boost::container::pmr::monotonic_buffer_resource> lmpool;


std::unique_ptr<std::vector<std::byte>> almbuffer;
std::unique_ptr<boost::container::pmr::monotonic_buffer_resource> almpool;

std::unique_ptr<std::vector<std::byte>> lmbufferouter;
std::unique_ptr<boost::container::pmr::monotonic_buffer_resource> lmpoolouter;


std::unique_ptr<std::vector<std::byte>> almbufferouter;
std::unique_ptr<boost::container::pmr::monotonic_buffer_resource> almpoolouter;


std::shared_ptr<std::vector<std::byte>> generatedChunksOnServerBuffer;
std::shared_ptr<boost::container::pmr::monotonic_buffer_resource> gcspool;



ChunkLightSources getChunkLightSourcesBlockAndAmbient(
    const TwoIntTup& spot, World* world, int chunkw, int chunkh,
    bool locked)
{
    std::vector<std::pair<IntTup, ColorPack>> oldChunkLightSources;
    std::vector<std::pair<IntTup, ColorPack>> chunkLightSources;
    std::vector<std::pair<IntTup, ColorPack>> oldAmbientLightSources;
    std::vector<std::pair<IntTup, ColorPack>> newAmbientLightSources;
    std::shared_lock<std::shared_mutex> url;
    std::shared_lock<std::shared_mutex> nrl;
    std::shared_lock<std::shared_mutex> lock3;
    if (!locked)
    {
        url = std::shared_lock<std::shared_mutex>(world->userDataMap.mutex());
        nrl = std::shared_lock<std::shared_mutex>(world->nonUserDataMap.mutex());
        lock3 = std::shared_lock<std::shared_mutex>(lightmapMutex);
    }
    auto startspot = IntTup(spot.x * chunkw, 0, spot.z * chunkw);
    for (int i = 0; i < chunkw; i++)
    {
        for (int k = 0; k < chunkw; k++)
        {
            bool foundground = false;
            bool hitsolid = false;
            for (int j = chunkh - 1; j > -1; j--)
            {
                auto spott = startspot + IntTup(i, j, k);
                auto h = world->getLocked(spott);
                auto originhash = IntTupHash{}(spott, true);
                if (h == LIGHT)
                {
                    chunkLightSources.push_back(std::make_pair(spott, TORCHLIGHTVAL));
                }
                auto ambientSpot = ambientlightmap.get(spott);
                if (ambientSpot != std::nullopt) {
                    auto& val = ambientSpot.value();
                    for (int i = 0; i < val->count; i++) {
                        if (originhash == val->originhashes[i]) {
                            oldAmbientLightSources.emplace_back(spott, SKYLIGHTVAL);
                        }
                    }
                }

                // Check block lightmap
                auto blockSpot = lightmap.get(spott);
                if (blockSpot != std::nullopt) {
                    auto& val = blockSpot.value();
                    for (int i = 0; i < val->count; i++) {
                        if (originhash == val->originhashes[i]) {
                            oldChunkLightSources.emplace_back(spott, TORCHLIGHTVAL);
                        }
                    }
                }
                if (h != AIR)
                {
                    if (!foundground)
                    {
                        newAmbientLightSources.push_back(std::make_pair(spott + IntTup(0, 1, 0), SKYLIGHTVAL));
                        foundground = true;
                    }
                    hitsolid = true;
                }
                else
                {

                    if (!hitsolid)
                    {
                        for (auto& neighb : neighbs4)
                        {
                            if (world->getLocked(neighb + spott) != AIR)
                            {
                                newAmbientLightSources.push_back(std::make_pair(spott, SKYLIGHTVAL));
                            }
                        }
                    }

                }

            }
        }
    }
    return ChunkLightSources{
        .oldBlockSources = oldChunkLightSources,
        .newBlockSources = chunkLightSources,
        .ambientOldSources = oldAmbientLightSources,
        .ambientNewSources = newAmbientLightSources,
    };
}



void propagateAllLightsLayered(World* world,
    const std::vector<std::pair<IntTup, ColorPack>>& lightSources,
    LightMapType& lightmap,
    TwoIntTup chunkOrigin,
    std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks,
    bool locked) {
    constexpr int maxLightLevel = 15; // Max value of any RGB component (0-15)
    constexpr int chunkWidth = 16, chunkHeight = 250, chunkDepth = 16;
    constexpr int pad = 16;
    constexpr size_t width = chunkWidth + 2 * pad;
    constexpr size_t height = chunkHeight + 2 * pad;
    constexpr size_t depth = chunkDepth + 2 * pad;
    constexpr size_t volume = width * height * depth;

    int minX = chunkOrigin.x * chunkWidth - pad;
    int minY = 0 - pad;
    int minZ = chunkOrigin.z * chunkDepth - pad;

    std::bitset<volume> visited;
    // Layers indexed by max(R,G,B)
    std::vector<std::vector<std::tuple<IntTup, IntTup, ColorPack>>> layers(maxLightLevel + 1);

    std::shared_lock<std::shared_mutex> url;
    std::shared_lock<std::shared_mutex> nrl;
    std::unique_lock<std::shared_mutex> lock3;
    if (!locked) {
        url = std::shared_lock<std::shared_mutex>(world->userDataMap.mutex());
        nrl = std::shared_lock<std::shared_mutex>(world->nonUserDataMap.mutex());
        lock3 = std::unique_lock<std::shared_mutex>(lightmapMutex);
    }

    // Initialize light sources
    for (const auto& [pos, color] : lightSources) {
        size_t idx = (pos.x - minX) + (pos.z - minZ) * width + (pos.y - minY) * width * depth;
        if (idx >= volume) continue;
        if (visited[idx]) continue;
        visited[idx] = true;

        // Use max(R,G,B) as the layer index
        int layer = std::max({ color.r(), color.g(), color.b() });
        layers[layer].push_back({ pos, pos, color });
        setLightLevelFromOriginHere<true>(pos, pos, color, lightmap);

        if (implicatedChunks) {
            TwoIntTup chunkPos = WorldRenderer::stupidWorldRendererWorldToChunkPos(TwoIntTup(pos.x, pos.z));
            if (chunkPos != TwoIntTup(chunkOrigin.x, chunkOrigin.z)) {
                implicatedChunks->insert(chunkPos);
            }
        }
    }



    // Propagate lights layer by layer
    for (int level = maxLightLevel; level > 0; --level) {
        for (const auto& [spot, source, color] : layers[level]) {
            for (const auto& offset : neighbs) {
                IntTup neighbor = spot + offset;
                size_t idx = (neighbor.x - minX) + (neighbor.z - minZ) * width + (neighbor.y - minY) * width * depth;
                if (idx >= volume) continue;
                if (visited[idx]) continue;
                if (!transparents.test(world->getLocked(neighbor))) continue;

                visited[idx] = true;

                //IF THE NEIGHBOR HAS NO NON-AIR/NON-TRULYNOTHING NEIGHBORS FUCK IT
                bool NEIGHBORHASATLEASTONESOLIDNEIGHBOR = false;
                for(const auto& newneigh : neighbs14)
                {
                    IntTup neighneigh = neighbor + newneigh;
                    if (!trulynothing.test(world->getLocked(neighneigh)))
                    {
                        NEIGHBORHASATLEASTONESOLIDNEIGHBOR = true;
                        break;
                    }
                }

                if(NEIGHBORHASATLEASTONESOLIDNEIGHBOR)
                {
                    // Reduce all RGB components by 1
                    ColorPack newColor = color - 1;
                    // Only propagate if newColor has non-zero components
                    if (newColor.r() > 0 || newColor.g() > 0 || newColor.b() > 0) {
                        int newLayer = std::max({ newColor.r(), newColor.g(), newColor.b() });
                        layers[newLayer].push_back({ neighbor, source, newColor });
                        setLightLevelFromOriginHere<true>(neighbor, source, newColor, lightmap);

                        if (implicatedChunks) {
                            TwoIntTup chunkPos = WorldRenderer::stupidWorldRendererWorldToChunkPos(TwoIntTup(neighbor.x, neighbor.z));
                            if (chunkPos != TwoIntTup(chunkOrigin.x, chunkOrigin.z)) {
                                implicatedChunks->insert(chunkPos);
                            }
                        }
                    }
                }

                
            }
        }
        layers[level].clear();
    }
}


void unpropagateAllLightsLayered(const std::vector<std::pair<IntTup, ColorPack>>& lightSourcesToRemove,
    LightMapType& lightmap,
    TwoIntTup chunkOrigin,
    std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks,
    bool locked) {
    constexpr int chunkWidth = 16, chunkHeight = 250, chunkDepth = 16;
    constexpr int pad = 16;
    constexpr size_t width = chunkWidth + 2 * pad;
    constexpr size_t height = chunkHeight + 2 * pad;
    constexpr size_t depth = chunkDepth + 2 * pad;
    constexpr size_t volume = width * height * depth;
    constexpr int maxLightLevel = 15; // Max value of any RGB component (0-15)

    int minX = chunkOrigin.x * chunkWidth - pad;
    int minY = 0 - pad;
    int minZ = chunkOrigin.z * chunkDepth - pad;

    std::bitset<volume> visited;
    // Layers indexed by distance from source (0 to maxLightLevel)
    std::vector<std::vector<std::pair<IntTup, IntTup>>> layers(maxLightLevel + 1);

    std::unique_lock<std::shared_mutex> lock;
    if (!locked) {
        lock = std::unique_lock<std::shared_mutex>(lightmapMutex);
    }

    // Initialize removal of light sources (layer 0)
    for (const auto& [pos, level] : lightSourcesToRemove) {
        size_t idx = (pos.x - minX) + (pos.z - minZ) * width + (pos.y - minY) * width * depth;
        if (idx >= volume) continue;
        if (visited[idx]) continue;
        visited[idx] = true;
        layers[0].push_back({ pos, pos });
        setLightLevelFromOriginHere(pos, pos, ColorPack(0, 0, 0), lightmap);
        if (implicatedChunks) {
            TwoIntTup chunkPos = WorldRenderer::stupidWorldRendererWorldToChunkPos(TwoIntTup(pos.x, pos.z));
            if (chunkPos != TwoIntTup(chunkOrigin.x, chunkOrigin.z)) {
                implicatedChunks->insert(chunkPos);
            }
        }
    }

    // Process layers outward (distance-based)
    for (int level = 0; level < maxLightLevel; ++level) {
        for (const auto& [spot, origin] : layers[level]) {
            auto originhash = IntTupHash{}(origin, true);
            for (const auto& offset : neighbs) {
                IntTup neighbor = spot + offset;
                size_t idx = (neighbor.x - minX) + (neighbor.z - minZ) * width + (neighbor.y - minY) * width * depth;
                if (idx >= volume) continue;
                if (visited[idx]) continue;

                auto lmentry = lightmap.get(neighbor);
                if (lmentry != std::nullopt) {
                    auto& val = lmentry.value();

                    if (val->deleteRay(originhash)) {
                        visited[idx] = true;
                        layers[level + 1].emplace_back( neighbor, origin );
                        //setLightLevelFromOriginHere(neighbor, origin, ColorPack(0, 0, 0), lightmap); Not needed?
                        if (implicatedChunks) {
                            TwoIntTup chunkPos = WorldRenderer::stupidWorldRendererWorldToChunkPos(TwoIntTup(neighbor.x, neighbor.z));
                            if (chunkPos != TwoIntTup(chunkOrigin.x, chunkOrigin.z)) {
                                implicatedChunks->insert(chunkPos);
                            }
                        }
                    }
                }
            }
        }
        layers[level].clear();
    }
}