//
// Created by jack on 2/9/2025.
//

#ifndef SCENE_H
#define SCENE_H
#include "PrecompHeader.h"
#include "world/WorldGizmo.h"
#include "world/gizmos/BlockSelectGizmo.h"
#include "Hud.h"
#include "Planets.h"
#include "world/gizmos/BulkPlaceGizmo.h"
#include "world/gizmos/VoxModelStampGizmo.h"


struct Settings
{
    float mouseSensitivity = 1.0f;

};
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
    BulkPlaceGizmo* bulkPlaceGizmo = nullptr;
    WorldRenderer* worldRenderer = nullptr;
    VoxModelStampGizmo* vmStampGizmo = nullptr;
    Hud* hud = nullptr;
    GLFWwindow* window = nullptr;
    std::string serverAddress = "127.0.0.1:6969";
    int currentSeed = 0;
    bool multiplayer = false;
    int rendDistSelection = 17;
    std::atomic<bool> worldReceived = {false};
    std::atomic<bool> clientShouldRun = {false};
    Settings settings = {};
    jl::Camera* guiCamera = nullptr;
    bool showingControls = false;
    std::string localServerPort = "";
    float timeOfDay = 0.0f;
    static constexpr float dayLength = 900.0f;
    PlanetType currentPlanetType = PlanetType::Home;




    void saveSettings()
    {
        std::ofstream settingsFile("settings.txt", std::ios::trunc);
        if (settingsFile.is_open())
        {
            settingsFile << settings.mouseSensitivity << "\n";
            settingsFile << serverAddress << "\n";
            settingsFile << rendDistSelection << "\n";
            settingsFile.close();
        }

    }
    void loadSettings()
    {
        std::ifstream settingsFile("settings.txt");
        if (settingsFile.is_open())
        {
            try
            {
                settingsFile >> settings.mouseSensitivity;
            } catch (const std::exception& e)
            {
                std::cout << "Couldn't load mouse sensitivity \n";
            }

            try
            {
                settingsFile >> serverAddress;
            } catch (const std::exception& e)
            {
                std::cout << "Couldn't load server address \n";
            }
            try
            {
                settingsFile >> rendDistSelection;
            } catch (std::exception& e)
            {
                std::cout << "Couldn't load render distance \n";
            }


        }
    }
    void enableMultiplayer()
    {
        multiplayer = true;
        clientShouldRun.store(true);
    }
};

extern Scene theScene;


#endif //SCENE_H
