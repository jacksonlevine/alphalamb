//
// Created by jack on 2/9/2025.
//

#ifndef SCENE_H
#define SCENE_H
#include "PrecompHeader.h"
#include "world/WorldGizmo.h"
#include "world/gizmos/BlockSelectGizmo.h"
#include "Hud.h"

struct Scene
{
    std::unordered_map<int, Player*> players = {};
    int myPlayerIndex = -1;
    size_t addPlayer()
    {
        int index = 0;
        while (players.contains(index))
        {
            index++;
        };
        players.insert({index, new Player()});
        return index;
    }
    size_t addPlayerWithIndex(size_t index)
    {
        players.insert_or_assign(index, new Player());
        return index;
    }
    bool mouseCaptured = false;
    bool firstMouse = true;
    std::vector<WorldGizmo*> gizmos;
    ParticlesGizmo* particles = nullptr;
    World* world = nullptr;
    BlockSelectGizmo* blockSelectGizmo = nullptr;
    WorldRenderer* worldRenderer = nullptr;
    Hud* hud = nullptr;
    GLFWwindow* window = nullptr;
    std::string serverAddress = "127.0.0.1:6969";
    int currentSeed = 0;
    bool multiplayer = false;
    std::atomic<bool> worldReceived = {false};
    std::atomic<bool> clientShouldRun = {false};
    void enableMultiplayer()
    {
        multiplayer = true;
        clientShouldRun.store(true);
    }
};

extern Scene theScene;


#endif //SCENE_H
