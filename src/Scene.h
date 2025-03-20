//
// Created by jack on 2/9/2025.
//

#ifndef SCENE_H
#define SCENE_H

#include "AmbOcclSetting.h"
#include "PrecompHeader.h"
#include "ClientUID.h"
#include "world/WorldGizmo.h"
#include "world/gizmos/BlockSelectGizmo.h"
#include "Hud.h"
#include "Planets.h"
#include "components/PlayerEmplacer.h"
#include "world/gizmos/BulkPlaceGizmo.h"
#include "world/gizmos/VoxModelStampGizmo.h"


struct Settings
{
    float mouseSensitivity = 1.0f;
    ClientUID clientUID;
    bool ambientOccl = false;
};
struct Scene
{
    //std::unordered_map<int, Player*> players = {};
    entt::registry REG = {};

    entt::entity myPlayerIndex = entt::null;


    template <typename Type>
    Type& our()
    {
        return REG.get<Type>(myPlayerIndex);
    }




    entt::entity addPlayerWithIndex(entt::entity index, ClientUID id)
    {
        auto pers = REG.create(index);

        emplacePlayerParts(REG, pers, id);


        return pers;
        // players.insert_or_assign(index, new Player());
        // return index;
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
    PlanetType currentPlanetType = PlanetType::Home;
    std::unordered_set<ClientUID, boost::hash<boost::uuids::uuid>> existingInvs = {};
    float worldIntroTimer = 0.0f;




    void saveSettings()
    {
        std::ofstream settingsFile("settings.txt", std::ios::trunc);
        if (settingsFile.is_open())
        {
            settingsFile << settings.mouseSensitivity << "\n";
            settingsFile << serverAddress << "\n";
            settingsFile << rendDistSelection << "\n";
            settingsFile << settings.clientUID << "\n";
            settingsFile << ambOccl << "\n";
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

                    settingsFile >> ambOccl;
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
