#include "Light.h"


#include "world/WorldRenderer.h"

template<bool queue>
void lightPassOnChunk(World* world, TwoIntTup spot, int chunkw, int chunkh,
                      bool
                      locked)
{
    auto implicatedChunks = std::unordered_set<TwoIntTup, TwoIntTupHash>();

    auto sources = getChunkLightSourcesBlockAndAmbient(spot, world, chunkw, chunkh, lightmap, locked);

    unpropagateAllLightsLayered(sources.first, lightmap, spot , &implicatedChunks, locked);
    propagateAllLightsLayered(world, sources.first, lightmap,spot , &implicatedChunks, locked);

    unpropagateAllLightsLayered(sources.second, ambientlightmap, spot ,&implicatedChunks, locked);
    propagateAllLightsLayered(world, sources.second, ambientlightmap,spot , &implicatedChunks, locked);
    if (queue)
    {
        for (const auto & spot2 : implicatedChunks)
        {

            lightOverlapNotificationQueue.push(spot2);

        }
    }

}
