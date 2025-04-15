//
// Created by jack on 4/12/2025.
//

#include "Light.h"

#include "world/WorldRenderer.h"
boost::sync_queue<TwoIntTup> lightOverlapNotificationQueue = {};




std::pair<std::vector<std::pair<IntTup, int>>, std::vector<std::pair<IntTup, int>>> getChunkLightSourcesBlockAndAmbient(
    const TwoIntTup& spot, World* world, int chunkw, int chunkh,
    LightMapType& lightmap, bool locked)
{
    std::vector<std::pair<IntTup, int>> chunkLightSources;
    std::vector<std::pair<IntTup, int>> ambientLightSources;
    std::shared_lock<std::shared_mutex> url;
    std::shared_lock<std::shared_mutex> nrl;
    std::shared_lock<std::shared_mutex> lock3;
    static IntTupHash spotHash;
    if (!locked)
    {
        url = std::shared_lock<std::shared_mutex>(world->userDataMap->mutex());
        nrl = std::shared_lock<std::shared_mutex>(world->nonUserDataMap->mutex());
    }
    auto startspot = IntTup(spot.x * chunkw, 0, spot.z * chunkw);
    for (int i = 0; i < chunkw; i++)
    {
        for (int k = 0; k < chunkw; k++)
        {
            bool foundground = false;
            std::pair<IntTup, int> spotgonnapush;
            for (int j = chunkh-1; j > -1; j--)
            {
                auto spott = startspot + IntTup(i,j,k);
                auto h = world->getLocked(spott);
                if (h == LIGHT)
                {
                    chunkLightSources.push_back(std::make_pair(spott, 12));
                }
                auto lightspot = ambientlightmap.find(spott);
                if (lightspot != ambientlightmap.end())
                {
                    for (const auto & ray : lightspot->second.rays)
                    {
                        if (spott == ray.origin)
                        {
                            spotgonnapush = std::make_pair(spott, 16);

                            foundground = true;
                        }
                    }

                }
                if (h != AIR && !foundground)
                {
                    spotgonnapush = std::make_pair(spott + IntTup(0,1,0), 16);
                    foundground = true;
                }
            }
            ambientLightSources.push_back(spotgonnapush);
        }

    }
    return std::make_pair(chunkLightSources, ambientLightSources);
}


void propagateAllLightsLayered(World* world,
                              const std::vector<std::pair<IntTup, int>>& lightSources,
                              LightMapType& lightmap,
                              TwoIntTup chunkOrigin,
                              std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks,
                              bool locked
                              ) {
    constexpr int maxLightLevel = 16;
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
    std::vector<std::vector<std::pair<IntTup, IntTup>>> layers(maxLightLevel + 1);
    std::array<bool, 256> transparentCache{};
    for (int i : transparents) {
        transparentCache[i] = true;
    }

    for (const auto& [pos, level] : lightSources) {
        size_t idx = (pos.x - minX) + (pos.z - minZ) * width + (pos.y - minY) * width * depth;
        if (idx >= volume) continue;
        if (visited[idx]) continue;
        visited[idx] = true;
        layers[level].push_back({pos, pos});
        setLightLevelFromOriginHere<true>(pos, pos, level, lightmap);
        if (implicatedChunks) {
            TwoIntTup chunkPos = WorldRenderer::worldToChunkPos(TwoIntTup(pos.x, pos.z));
            if (chunkPos != TwoIntTup(chunkOrigin.x, chunkOrigin.z)) {
                implicatedChunks->insert(chunkPos);
            }
        }
    }

    std::shared_lock<std::shared_mutex> url;
    std::shared_lock<std::shared_mutex> nrl;
    std::unique_lock<std::shared_mutex> lock3;
    if (!locked) {
        url = std::shared_lock<std::shared_mutex>(world->userDataMap->mutex());
        nrl = std::shared_lock<std::shared_mutex>(world->nonUserDataMap->mutex());
        lock3 = std::unique_lock<std::shared_mutex>(lightmapMutex);
    }

    for (int level = maxLightLevel; level > 1; --level) {
        for (const auto& [spot, source] : layers[level]) {
            for (const auto& offset : neighbs) {
                IntTup neighbor = spot + offset;
                size_t idx = (neighbor.x - minX) + (neighbor.z - minZ) * width + (neighbor.y - minY) * width * depth;
                if (idx >= volume) continue;
                if (visited[idx]) continue;
                if (!transparentCache[world->getLocked(neighbor)]) continue;

                visited[idx] = true;
                layers[level - 1].push_back({neighbor, source});
                setLightLevelFromOriginHere<true>(neighbor, source, level - 1, lightmap);
                if (implicatedChunks) {
                    TwoIntTup chunkPos = WorldRenderer::worldToChunkPos(TwoIntTup(neighbor.x, neighbor.z));
                    if (chunkPos != TwoIntTup(chunkOrigin.x, chunkOrigin.z)) {
                        implicatedChunks->insert(chunkPos);
                    }
                }
            }
        }
        layers[level].clear();
    }
}
void unpropagateAllLightsLayered(const std::vector<std::pair<IntTup, int>>& lightSourcesToRemove,
                                 LightMapType& lightmap,

                                 TwoIntTup chunkOrigin,

                                 std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks,
                                 bool locked) { // Added parameter
    constexpr int maxLightLevel = 16;
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
    std::vector<std::vector<std::pair<IntTup, IntTup>>> layers(maxLightLevel + 1);

    std::unique_lock<std::shared_mutex> lock;
    if (!locked) {
        lock = std::unique_lock<std::shared_mutex>(lightmapMutex);
    }
    IntTupHash hasher;
    for (const auto& [pos, level] : lightSourcesToRemove) {
        size_t idx = (pos.x - minX) + (pos.z - minZ) * width + (pos.y - minY) * width * depth;
        if (idx >= volume) continue;
        if (visited[idx]) continue;
        visited[idx] = true;
        layers[level].push_back({pos, pos});
        setLightLevelFromOriginHere(pos, pos, 0, lightmap);
        if (implicatedChunks) {
            TwoIntTup chunkPos = WorldRenderer::worldToChunkPos(TwoIntTup(pos.x, pos.z));
            if (chunkPos != TwoIntTup(chunkOrigin.x, chunkOrigin.z)) {
                implicatedChunks->insert(chunkPos);
            }
        }
    }

    for (int level = maxLightLevel; level > 1; --level) {
        for (const auto& [spot, origin] : layers[level]) {
            for (const auto& offset : neighbs) {
                IntTup neighbor = spot + offset;
                size_t idx = (neighbor.x - minX) + (neighbor.z - minZ) * width + (neighbor.y - minY) * width * depth;
                if (idx >= volume) continue;
                if (visited[idx]) continue;

                auto lmentry = lightmap.find(neighbor);
                if (lmentry != lightmap.end()) {
                    auto& rays = lmentry->second.rays;
                    for (auto it = rays.begin(); it != rays.end();) {
                        if (it->origin == origin) {
                            it = rays.erase(it);
                        } else {
                            ++it;
                        }
                    }
                    if (rays.empty()) {
                        lightmap.erase(lmentry);
                    } else {
                        visited[idx] = true;
                        layers[level - 1].push_back({neighbor, origin});
                        setLightLevelFromOriginHere(neighbor, origin, 0, lightmap);
                        if (implicatedChunks) {
                            TwoIntTup chunkPos = WorldRenderer::worldToChunkPos(TwoIntTup(neighbor.x, neighbor.z));
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
