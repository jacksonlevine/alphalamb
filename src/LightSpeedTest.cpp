//
// Created by jack on 4/13/2025.
//

#include "LightSpeedTest.h"

#include "world/WorldRenderer.h"

void lighttest()
{

    World world(new HashMapDataMap(), new OverworldWorldGenMethod(), new HashMapDataMap());
    auto lm = LightMapType();

    auto timebefore = std::chrono::high_resolution_clock::now();

    // for(int i = 0; i < 5; i++) {
    //     for(int j = 0; j < 5; j++) {
    //             for(int k = 0; k < 5; k++) {
    //                 propagateLight(IntTup(0,0,0));
    //             }
    //         }
    // }
    int fake = 0;
    std::vector<std::pair<IntTup, int>>* lightSources = new std::vector<std::pair<IntTup, int>>();
    {
        auto url = std::shared_lock<std::shared_mutex>(world.userDataMap->mutex());
        auto nrl = std::shared_lock<std::shared_mutex>(world.nonUserDataMap->mutex());
        auto lock3 = std::shared_lock<std::shared_mutex>(lightmapMutex);

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 250; j++) {
                for (int k = 0; k < 16; k++) {

                    if (world.getLocked(IntTup(i, j, k) ) == LIGHT)
                    {
                        fake++;
                    }
                    lightSources->emplace_back(IntTup(i, j, k), 16);
                }
            }
        }
    }

    propagateAllLightsLayered(&world, *lightSources, lm, nullptr, false);

    auto timeafter = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timeafter - timebefore).count();

    std::cout << "elapsed: " << elapsed << "ms" << std::endl;
}
