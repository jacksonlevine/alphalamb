//
// Created by jack on 2/9/2025.
//

#ifndef SCENE_H
#define SCENE_H

#include "PrecompHeader.h"
#include "ClientUID.h"
#include "world/WorldGizmo.h"
#include "world/gizmos/BlockSelectGizmo.h"
#include "Hud.h"
#include "Planets.h"
#include "world/gizmos/BulkPlaceGizmo.h"
#include "world/gizmos/VoxModelStampGizmo.h"


struct Settings
{
    float mouseSensitivity = 1.0f;
    ClientUID clientUID;
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
    float timeOfDay = 300.0f;
    static constexpr float dayLength = 900.0f;
    PlanetType currentPlanetType = PlanetType::Hostile;
    std::unordered_set<ClientUID, boost::hash<boost::uuids::uuid>> existingInvs = {};



    void saveSettings()
    {
        std::ofstream settingsFile("settings.txt", std::ios::trunc);
        if (settingsFile.is_open())
        {
            settingsFile << settings.mouseSensitivity << "\n";
            settingsFile << serverAddress << "\n";
            settingsFile << rendDistSelection << "\n";
            settingsFile << settings.clientUID << "\n";
            settingsFile.close();
        }

    }
    void loadSettings()
    {
        if (std::filesystem::exists("settings.txt"))
        {
            std::ifstream settingsFile("settings.txt");
            if (settingsFile.is_open())
            {

                    settingsFile >> settings.mouseSensitivity;

                    settingsFile >> serverAddress;

                    settingsFile >> rendDistSelection;

                    settingsFile >> settings.clientUID;
                    std::cout << "Loaded UID: " << settings.clientUID << std::endl;
            }
        } else
        {
            settings.clientUID = boost::uuids::random_generator()();
            std::cout << "New UID: " << settings.clientUID << std::endl;
            saveSettings();
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
