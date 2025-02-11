//
// Created by jack on 1/27/2025.
//




#include "BasicShader.h"
#include "Player.h"
#include "Shader.h"
#include "Texture.h"

#include "PrecompHeader.h"


#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>

#include "Client.h"
#include "HandledBlock.h"
#include "PhysXStuff.h"
#include "Sky.h"
#include "world/SWCLoader.h"
#include "world/World.h"
#include "world/WorldGizmo.h"
#include "world/WorldRenderer.h"
#include "world/gizmos/BlockSelectGizmo.h"
#include "world/datamapmethods/HashMapDataMap.h"
#include "world/gizmos/ParticlesGizmo.h"
#include "world/worldgenmethods/OverworldWorldGenMethod.h"
#include "ImGuiStuff.h"
#include "LUTLoader.h"
#include "Hud.h"
#include "SharedVarsBetweenMainAndGui.h"
#include "Texture2DArray.h"

boost::asio::io_context io_context;
boost::asio::ip::tcp::socket tsocket(io_context);
boost::asio::ip::tcp::resolver resolver(io_context);

Scene theScene = {};

constexpr double J_PI = 3.1415926535897932384626433832;
constexpr double DEG_TO_RAD = J_PI / 180.0;

int properMod(int a, int b) {
    int m = a % b;
    return m < 0 ? m + b : m;
}

void sendControlsUpdatesLol(tcp::socket& socket, float deltaTime)
{
    static float timer = 0.0f;

    auto& player = theScene.players.at(theScene.myPlayerIndex);
    static Controls lastcontrols = player->controls;
    if(player->controls != lastcontrols)
    {
        lastcontrols = player->controls;
        DGMessage cu = ControlsUpdate(theScene.myPlayerIndex, player->controls, player->camera.transform.position, glm::vec2(player->camera.transform.yaw, player->camera.transform.pitch));
        pushToMainToNetworkQueue(cu);

    }

    if (timer > 0.2)
    {
        timer = 0.0f;


        static float lastpitch = player->camera.transform.pitch;
        static float lastyaw = player->camera.transform.yaw;

        if(lastpitch != player->camera.transform.pitch || lastyaw != player->camera.transform.yaw)
        {
            lastpitch = player->camera.transform.pitch;
            lastyaw = player->camera.transform.yaw;

            DGMessage player_yaw_pitch_update = YawPitchUpdate {
                theScene.myPlayerIndex,
                lastyaw,
                lastpitch
            };
            pushToMainToNetworkQueue(player_yaw_pitch_update);


        }
    } else
    {
        timer += deltaTime;
    }

}



void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
    if(action)
    {
        if(button == GLFW_MOUSE_BUTTON_LEFT)
        {

            if(!scene->mouseCaptured)
            {
                if(imguiio->WantCaptureMouse)
                {
                    return;
                }
                if (currentGuiScreen == GuiScreen::InGame)
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    scene->mouseCaptured = true;
                }
            } else
            {
                //addShootLine();
                //sendShootLineMessage();
                auto & cam = scene->players[scene->myPlayerIndex]->camera;


                if (scene->multiplayer)
                {
                    auto & spot = scene->blockSelectGizmo->selectedSpot;
                    //std::cout << "Senfing blokc chagne \n";
                    pushToMainToNetworkQueue(BlockSet{
                        spot, AIR
                    });

                } else
                {
                    if (scene->world && scene->blockSelectGizmo && scene->worldRenderer)
                {
                    //std::cout << "Setting" << std::endl;
                    auto & spot = scene->blockSelectGizmo->selectedSpot;
                    //std::cout << "At Spot: " << spot.x << ", " << spot.y << ", " << spot.z << std::endl;
                    uint32_t blockThere = scene->world->get(spot);
                    glm::vec3 burstspot = glm::vec3(
                        scene->blockSelectGizmo->selectedSpot.x+ 0.5,
                        scene->blockSelectGizmo->selectedSpot.y + 0.5,
                        scene->blockSelectGizmo->selectedSpot.z + 0.5);
                    if (scene->particles)
                    {
                        scene->particles->particleBurst(burstspot,
                                                     20, (MaterialName)blockThere, 2.0, 1.0f);
                    }

                    //scene->world->set(spot, AIR);
    //std::cout << "Set the block "  << std::endl;;
                    auto cs = scene->worldRenderer->chunkSize;
                    // Then in your code:
                    auto xmod = properMod(spot.x, cs);
                    auto zmod = properMod(spot.z, cs);
                    //std::cout << "Xmod: " << xmod << " Zmod: " << zmod << std::endl;
                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                        spot, AIR, false
                        );

                    if(xmod == scene->worldRenderer->chunkSize - 1)
                    {
                        scene->worldRenderer->requestChunkRebuildFromMainThread(
                            IntTup(spot.x+1, spot.y, spot.z));

                    } else if(xmod == 0)
                    {
                        scene->worldRenderer->requestChunkRebuildFromMainThread(
                            IntTup(spot.x-1, spot.y, spot.z));
                    }

                    if(zmod == scene->worldRenderer->chunkSize - 1)
                    {
                        scene->worldRenderer->requestChunkRebuildFromMainThread(
                            IntTup(spot.x, spot.y, spot.z+1));

                    } else if(zmod == 0)
                    {
                        scene->worldRenderer->requestChunkRebuildFromMainThread(
                            IntTup(spot.x, spot.y, spot.z-1));
                    }
                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                        spot
                        );

                    //std::cout << "Request filed " << std::endl;
                }
                }

            }

        }
        else if(button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            auto & cam = scene->players[scene->myPlayerIndex]->camera;


            if (scene->multiplayer)
            {
                auto & spot = scene->blockSelectGizmo->selectedSpot;
                IntTup placeSpot = scene->blockSelectGizmo->selectedSpot + scene->blockSelectGizmo->hitNormal;

                    using namespace std;

                IntTup playerBlockSpot1 = IntTup(floor(cam.transform.position.x), floor(cam.transform.position.y), floor(cam.transform.position.z));
                IntTup playerBlockSpot2 = IntTup(floor(cam.transform.position.x), floor(cam.transform.position.y-1), floor(cam.transform.position.z));

                if (placeSpot != playerBlockSpot1 && placeSpot != playerBlockSpot2)
                {
                    //std::cout << "Senfing blokc place \n";
                    pushToMainToNetworkQueue(BlockSet{
                        placeSpot, scene->players.at(scene->myPlayerIndex)->currentHeldBlock
                    });
                }


            } else
            {
                if (scene->world && scene->blockSelectGizmo && scene->worldRenderer)
                {
                    //std::cout << "Setting" << std::endl;
                    auto & spot = scene->blockSelectGizmo->selectedSpot;
                    //std::cout << "At Spot: " << spot.x << ", " << spot.y << ", " << spot.z << std::endl;
                    //uint32_t blockThere = scene->world->get(spot);
                    IntTup placeSpot = scene->blockSelectGizmo->selectedSpot + scene->blockSelectGizmo->hitNormal;

                    //scene->world->set(spot, AIR);
                    //std::cout << "Set the block "  << std::endl;;
                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                        placeSpot, scene->players.at(scene->myPlayerIndex)->currentHeldBlock
                        );


                    //std::cout << "Request filed " << std::endl;
                }
            }


        } else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            if (scene->world && scene->blockSelectGizmo && scene->worldRenderer && scene->myPlayerIndex != -1)
            {
                if (scene->blockSelectGizmo->isDrawing)
                {
                    //std::cout << "Setting" << std::endl;
                    auto & spot = scene->blockSelectGizmo->selectedSpot;
                    //std::cout << "At Spot: " << spot.x << ", " << spot.y << ", " << spot.z << std::endl;
                    uint32_t blockThere = scene->world->get(spot);
                    //IntTup placeSpot = scene->blockSelectGizmo->selectedSpot + scene->blockSelectGizmo->hitNormal;
                    scene->players.at(scene->myPlayerIndex)->currentHeldBlock = (MaterialName)blockThere;
                    //scene->world->set(spot, AIR);
                    //std::cout << "Set the block "  << std::endl;;
                    // scene->worldRenderer->requestChunkRebuildFromMainThread(
                    //     placeSpot, scene->players.at(scene->myPlayerIndex)->currentHeldBlock
                    //     );


                } else
                {
                    scene->players.at(scene->myPlayerIndex)->currentHeldBlock = AIR;
                }

                //std::cout << "Request filed " << std::endl;
            }
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {


    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));

    if (!scene->mouseCaptured)
    {
        if (imguiio->WantCaptureKeyboard) return;
    }
    if (scene->myPlayerIndex != -1)
    {
        // if (key == GLFW_KEY_F)
        // {
        //     if(action == GLFW_PRESS)
        //     {
        //         std::cout << "Toggling fly mode \n";
        //         FLY_MODE = !FLY_MODE;
        //     }
        //
        // }
        if (key == GLFW_KEY_L)
        {
            scene->world->save("testfile.txt");
        } else
        if(key == GLFW_KEY_ESCAPE && scene->mouseCaptured == true)
        {
            //glfwSetWindowShouldClose(window, 1);
            currentGuiScreen = GuiScreen::EscapeMenu;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            scene->mouseCaptured = false;
            scene->firstMouse = true;
        }
        if (key == GLFW_KEY_P)
        {




            scene->world->setSeed(time(NULL));
            scene->worldRenderer->launchThreads(&scene->players.at(scene->myPlayerIndex)->camera, scene->world);
        }else
            if (key == GLFW_KEY_T)
            {
                std::cout << "Num threads running: " << NUM_THREADS_RUNNING << '\n';
            } else
        if (key == GLFW_KEY_E)
        {
            scene->players.at(scene->myPlayerIndex)->controls.secondary2 = action;
        } else
        if (key == GLFW_KEY_LEFT_SHIFT)
        {
            scene->players.at(scene->myPlayerIndex)->controls.sprint = action;
        } else
        if (key == GLFW_KEY_F)
        {
            scene->players.at(scene->myPlayerIndex)->controls.secondary1 = action;
            scene->players.at(scene->myPlayerIndex)->controls.jump = action;
        } else
        if (key == GLFW_KEY_W)
        {
            scene->players.at(scene->myPlayerIndex)->controls.forward = action;
        } else
        if (key == GLFW_KEY_A)
        {
            scene->players.at(scene->myPlayerIndex)->controls.left = action;
        } else
        if (key == GLFW_KEY_S)
        {
            scene->players.at(scene->myPlayerIndex)->controls.backward = action;
        } else
        if (key == GLFW_KEY_D)
        {
            scene->players.at(scene->myPlayerIndex)->controls.right = action;
        } else
        if (key == GLFW_KEY_SPACE)
        {
            scene->players.at(scene->myPlayerIndex)->controls.jump = action;
        }
    }
}
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    static double lastx = 0.0;
    static double lasty = 0.0;
    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
    if(scene->mouseCaptured)
    {

        if(scene->firstMouse)
        {
            lastx = xpos;
            lasty = ypos;
            scene->firstMouse = false;
        }


        //ADD SENSITIVITY HERE
        const double xOffset = (xpos - lastx) * (scene->settings.mouseSensitivity * 0.3);
        const double yOffset = (lasty - ypos) * (scene->settings.mouseSensitivity * 0.3);

        //std::cout << "Yaw: " << std::to_string(CAMERA.transform.yaw) << " Pitch: " << std::to_string(CAMERA.transform.pitch) << "\n";

        if (scene->myPlayerIndex != -1)
        {
            // print(cout, "xOffset: {}, yOffset: {} \n", xOffset, yOffset);
            auto & camera = scene->players.at(scene->myPlayerIndex)->camera;

            camera.setYawPitch(camera.transform.yaw + static_cast<float>(xOffset), camera.transform.pitch + static_cast<float>(yOffset));
        }

        lastx = xpos;
        lasty = ypos;
    }
}

void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
    if (scene->myPlayerIndex != -1)
    {
        auto & camera = scene->players.at(scene->myPlayerIndex)->camera;
        camera.updateProjection(width, height, 90.0f);
    }

    if(scene->hud)
    {
        Hud::windowWidth = width;
        Hud::windowHeight = height;
        scene->hud->uploaded = false;
        scene->hud->rebuildDisplayData();
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    static float lastTime = glfwGetTime();
    if (glfwGetTime() - lastTime > 0.07f)
    {
        lastTime = glfwGetTime();
        Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
        if (scene->myPlayerIndex != -1)
        {

            if (yoffset > 0)
            {
                scene->players.at(scene->myPlayerIndex)->currentHeldBlock = (MaterialName)(((int)scene->players.at(scene->myPlayerIndex)->currentHeldBlock + 1 ) % BLOCK_COUNT);
            }
            if (yoffset < 0)
            {
                int newMat = (scene->players.at(scene->myPlayerIndex)->currentHeldBlock - 1) % BLOCK_COUNT;
                if (newMat < 0)
                {
                    newMat = BLOCK_COUNT - 1;
                }
                scene->players.at(scene->myPlayerIndex)->currentHeldBlock = (MaterialName)newMat;
            }

            if (scene->multiplayer)
            {
                DGMessage sbu = PlayerSelectBlockChange{
                    scene->myPlayerIndex,
                scene->players.at(scene->myPlayerIndex)->currentHeldBlock};
                pushToMainToNetworkQueue(sbu);
            }
        }
    }

}



void enterWorld(Scene* s)
{

    int width, height = 0;
    glfwGetWindowSize(s->window, &width, &height);

    if (!s->multiplayer)
    {
        //std::cout << " This happening \n";
        s->myPlayerIndex = s->addPlayer();
    }
    //Add ourselves to the scene


    {
        auto & players = s->players;
        //std::cout << "Player index on entering: " << s->myPlayerIndex;
        auto & player = players.at(s->myPlayerIndex);
        auto & camera = player->camera;
        camera.updateProjection(width, height, 90.0f);
    }

    s->worldRenderer->launchThreads(&s->players.at(s->myPlayerIndex)->camera, s->world);
}

int main()
{

    glfwInit();
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    theScene.window = glfwCreateWindow(800, 800, "project7", nullptr, nullptr);
    GLFWwindow* window = theScene.window;
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
    };

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
    glfwSwapInterval(0);

    initializePhysX();

    loadGameVoxelModels();

    GLuint lutTexture = load3DLUT("resources/film_default.png");

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);


    float deltaTime = 0.0f;

    glfwSetWindowUserPointer(window, &theScene);

    initializeImGui(window);
    ImFont* font_title = imguiio->Fonts->AddFontFromFileTTF("font.ttf", 20.0f, NULL, imguiio->Fonts->GetGlyphRangesDefault());

    ImFont* font_body = imguiio->Fonts->AddFontFromFileTTF("font.ttf", 20.0f, NULL, imguiio->Fonts->GetGlyphRangesDefault());



    theScene.loadSettings();


    ParticlesGizmo* particles = new ParticlesGizmo();
    theScene.particles = particles;
    theScene.gizmos.push_back(particles);

    BlockSelectGizmo* bsg = new BlockSelectGizmo();
    theScene.blockSelectGizmo = bsg;
    theScene.gizmos.push_back(bsg);

    for(auto & gizmo : theScene.gizmos)
    {
        gizmo->init();
    }

    World world(
        new HashMapDataMap(),
        new OverworldWorldGenMethod(),
        new HashMapDataMap());

    theScene.world = &world;
    VoxModel swcModel = loadSwc("resources/swctest.txt");
    stampVoxelModelInWorld(&world,swcModel);

    theScene.hud = new Hud();
    theScene.hud->rebuildDisplayData();

    WorldRenderer* renderer = new WorldRenderer();

    theScene.worldRenderer = renderer;

    //theScene.addPlayerWithIndex(99);

    jl::prepareBillboard();

    std::vector<std::string> paths = {"resources/sprite/idle.png", "resources/sprite/run.png", "resources/sprite/jump.png", "resources/sprite/leftstrafe.png", "resources/sprite/rightstrafe.png", "resources/sprite/backwardsrun.png"};
    jl::Texture2DArray texture2DArray(paths);
    texture2DArray.bind(2);

    static jl::Shader billboardInstShader = getBillboardInstanceShader();


    while (!glfwWindowShouldClose(window))
    {
        static float lastTime = glfwGetTime();
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        glBindVertexArray(billboardVAO);
        glUseProgram(billboardInstShader.shaderID);



        static jl::Shader gltfShader = getBasicShader();
        static jl::Texture worldTex("resources/world.png");



        if (theScene.myPlayerIndex != -1)
        {

            std::vector<Billboard> billboards;
            std::vector<AnimationState> animStates;
            billboards.reserve(theScene.players.size());
            animStates.reserve(theScene.players.size());

            if(theScene.multiplayer)
            {
                sendControlsUpdatesLol(tsocket, deltaTime);
            }

            for (auto & [id, player] : theScene.players)
        {




            player->billboard.position = player->camera.transform.position;
            player->billboard.direction = player->camera.transform.direction;
                player->billboard.characterNum = id % 4;


            // std::cout << "Playerindex: " << id << "\n";
            // std::cout << "PositionCA: " << player->camera.transform.position.x << " " << player->camera.transform.position.y << " " << player->camera.transform.position.z << " \n";
            // std::cout << "PositionBI: " << player->billboard.position.x << " " << player->billboard.position.y << " " << player->billboard.position.z << " \n";
            // auto contpos = player->controller->getPosition();
            // std::cout << "PositionCO: " << contpos.x << " " << contpos.y << " " << contpos.z << " \n";
            // std::cout << player->controls << "\n";
            // std::cout << "Billboard Information:" << std::endl;
            // std::cout << "  Position: ("
            //           << player->billboard.position.x << ", "
            //           << player->billboard.position.y << ", "
            //           << player->billboard.position.z << ")" << std::endl;
            // std::cout << "  Direction: ("
            //           << player->billboard.direction.x << ", "
            //           << player->billboard.direction.y << ", "
            //           << player->billboard.direction.z << ")" << std::endl;
            // std::cout << "  Character Number: " << player->billboard.characterNum << std::endl;
            // std::cout << "  Hidden: " << player->billboard.hidden << std::endl;
            //
            // // Print AnimationState
            // std::cout << "\nAnimation State Information:" << std::endl;
            // std::cout << "  Action Number: " << player->animation_state.actionNum << std::endl;
            // std::cout << "  Time Started: " << player->animation_state.timestarted << std::endl;
            // std::cout << "  Time Scale: " << player->animation_state.timescale << std::endl;

            player->collisionCage.updateToSpot(&world, player->camera.transform.position, deltaTime);
                player->camera.updateWithYawPitch(player->camera.transform.yaw, player->camera.transform.pitch);

            if(id != theScene.myPlayerIndex)
            {
                billboards.push_back(player->billboard);
                animStates.push_back(player->animation_state);
            }


            float wantedAnim = IDLE;
            if(player->controls.anyMovement())
            {
                if(player->controls.jump)
                {
                    wantedAnim = JUMP;
                }
                else
                    if(player->controls.left && !player->controls.right)
                    {
                        wantedAnim = LEFTSTRAFE;
                    } else
                        if(player->controls.right && !player->controls.left)
                        {
                            wantedAnim = RIGHTSTRAFE;
                        } else
                            if(player->controls.backward && !player->controls.forward)
                            {
                                wantedAnim = BACKWARDSRUN;
                            } else
                            {
                                wantedAnim = RUN;
                            }
            }

            auto existinganim = &player->animation_state;
            AnimationState newState{
                wantedAnim,
                (float)glfwGetTime(),
                wantedAnim == JUMP ? 0.5f : 1.0f
            };

            bool jumpinprogress = (existinganim->actionNum == JUMP) && (glfwGetTime() - existinganim->timestarted < 0.75f);
            if(existinganim->actionNum != wantedAnim && !jumpinprogress)
            {
                player->animation_state = newState;
            }
            player->update(deltaTime, &world, particles);
        }
        jl::updateBillboards(billboards);
        jl::updateAnimStates(animStates);




            DGMessage msg = {};

            while (networkToMainBlockChangeQueue.pop(&msg))
            {



                visit([&](const auto& m) {
                    using T = std::decay_t<decltype(m)>;
                    if constexpr (std::is_same_v<T, WorldInfo>) {
                        std::cout << "Got world info " << m.seed << " \n"
                        << "playerIndex: " << m.yourPlayerIndex << " \n"
                        << "yourPosition: " << m.yourPosition.x << " " << m.yourPosition.y << " " << m.yourPosition.z << " \n";

                        theScene.world->setSeed(m.seed);
                        theScene.myPlayerIndex = theScene.addPlayerWithIndex(m.yourPlayerIndex);

                        //Dont do this yet, receive the file first
                        //theScene.worldReceived = true;
                    }
                    else if constexpr (std::is_same_v<T, ControlsUpdate>) {
                        // std::cout << "Got a controls update from " << m.myPlayerIndex << "\n";
                        // std::cout << m.myControls << " \n";
                        // std::cout << m.startPos.x << " " << m.startPos.y << " " << m.startPos.z << "\n";
                        if(!theScene.players.contains(m.myPlayerIndex))
                        {
                            theScene.addPlayerWithIndex(m.myPlayerIndex);
                        }
                        theScene.players.at(m.myPlayerIndex)->controls = m.myControls;
                        theScene.players.at(m.myPlayerIndex)->camera.transform.position = m.startPos;
                        theScene.players.at(m.myPlayerIndex)->controller->setPosition(PxExtendedVec3(
                    m.startPos.x,
                    m.startPos.y - CAMERA_OFFSET,
                    m.startPos.z)
                        );

                        theScene.players.at(m.myPlayerIndex)->camera.transform.yaw = m.startYawPitch.x;
                        theScene.players.at(m.myPlayerIndex)->camera.transform.pitch = m.startYawPitch.y;

                    }
                    else if constexpr (std::is_same_v<T, YawPitchUpdate>)
                    {
                        if(!theScene.players.contains(m.myPlayerIndex))
                        {
                            theScene.addPlayerWithIndex(m.myPlayerIndex);
                        }
                        theScene.players.at(m.myPlayerIndex)->camera.updateWithYawPitch(m.newYaw, m.newPitch);

                    }
                    else if constexpr (std::is_same_v<T, PlayerSelectBlockChange>)
                    {
                        if(theScene.players.contains(m.myPlayerIndex))
                        {
                            theScene.players.at(m.myPlayerIndex)->currentHeldBlock = m.newMaterial;
                        }
                    }
                    else if constexpr (std::is_same_v<T, PlayerLeave>)
                    {
                        if(theScene.players.contains(m.myPlayerIndex))
                        {
                            theScene.players.erase(m.myPlayerIndex);
                        }
                    }
                    else if constexpr (std::is_same_v<T, PlayerPresent>) {
                       std::cout << "Processing palyerpresent\n";
                        theScene.addPlayerWithIndex(m.index);
                        theScene.players.at(m.index)->controller->setPosition(PxExtendedVec3(
                        m.position.x,
                        m.position.y + CAMERA_OFFSET,
                        m.position.z)
                            );
                        theScene.players.at(m.index)->camera.transform.position = m.position;
                        theScene.players.at(m.index)->camera.transform.direction = m.direction;
                    }
                    else if constexpr (std::is_same_v<T, BlockSet>) {
                        std::cout << "Processing network block change \n";

                        Scene* scene = &theScene;
                        //std::cout << " Server Setting" << std::endl;
                            auto & spot = m.spot;
                            //std::cout << "At Spot: " << spot.x << ", " << spot.y << ", " << spot.z << std::endl;
                            uint32_t blockThere = scene->world->get(spot);
                            glm::vec3 burstspot = glm::vec3(
                                spot.x+ 0.5,
                                spot.y + 0.5,
                                spot.z + 0.5);
                            if (scene->particles)
                            {
                                scene->particles->particleBurst(burstspot,
                                                             20, (MaterialName)blockThere, 2.0, 1.0f);
                            }

                            if (m.block == AIR)
                            {
                                auto cs = scene->worldRenderer->chunkSize;
                                // Then in your code:
                                auto xmod = properMod(spot.x, cs);
                                auto zmod = properMod(spot.z, cs);

                                scene->worldRenderer->requestChunkRebuildFromMainThread(
                                    spot, AIR, false
                                    );


                                if(xmod == scene->worldRenderer->chunkSize - 1)
                                {
                                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                                        IntTup(spot.x+1, spot.y, spot.z));

                                } else if(xmod == 0)
                                {
                                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                                        IntTup(spot.x-1, spot.y, spot.z));
                                }

                                if(zmod == scene->worldRenderer->chunkSize - 1)
                                {
                                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                                        IntTup(spot.x, spot.y, spot.z+1));

                                } else if(zmod == 0)
                                {
                                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                                        IntTup(spot.x, spot.y, spot.z-1));
                                }
                                scene->worldRenderer->requestChunkRebuildFromMainThread(
                                    spot
                                    );

                            } else
                            {
                                scene->worldRenderer->requestChunkRebuildFromMainThread(
                                    spot, m.block
                                    );
                            }
                            }
                            else if constexpr (std::is_same_v<T, FileTransferInit>) {

                            }
                        }, msg);



                break; //Only do one per frame
            }





            gScene->simulate(deltaTime);


            auto & camera = theScene.players.at(theScene.myPlayerIndex)->camera;


            drawSky(glm::vec4(0.3, 0.65, 1.0, 1.0),
                    glm::vec4(1.0, 1.0, 1.0, 1.0),
                    1.0f, &theScene.players[theScene.myPlayerIndex]->camera, lutTexture);


            glUseProgram(gltfShader.shaderID);

            static GLuint mvpLoc = glGetUniformLocation(gltfShader.shaderID, "mvp");
            static GLuint texture1Loc = glGetUniformLocation(gltfShader.shaderID, "texture1");
            static GLuint lutLoc = glGetUniformLocation(gltfShader.shaderID, "lut");
            static GLuint posLoc = glGetUniformLocation(gltfShader.shaderID, "pos");
            static GLuint rotLoc = glGetUniformLocation(gltfShader.shaderID, "rot");
            static GLuint camPosLoc = glGetUniformLocation(gltfShader.shaderID, "camPos");
            static GLuint grcLoc = glGetUniformLocation(gltfShader.shaderID, "grassRedChange");
            static GLuint scaleLoc = glGetUniformLocation(gltfShader.shaderID, "scale");
            static GLuint timeRenderedLoc = glGetUniformLocation(gltfShader.shaderID, "timeRendered");
            static GLuint offsetLoc = glGetUniformLocation(gltfShader.shaderID, "offs");

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(camera.mvp));
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(texture1Loc, 0);
            glUniform3f(posLoc, 0.0, 0.0, 0.0);
            const glm::vec3 campos = theScene.players[theScene.myPlayerIndex]->camera.transform.position;
            glUniform3f(camPosLoc, campos.x, campos.y, campos.z);
            glUniform1f(rotLoc, 0.0f);
            glUniform3f(offsetLoc, 0.0f, 0.0f, 0.0f);
            glUniform1f(scaleLoc, 1.0f);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_3D, lutTexture);
            glUniform1i(lutLoc, 1);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, worldTex.id);

            if(FLY_MODE)
            {
                std::cout << "" << camera.transform.position.x << " " << camera.transform.position.y << " " << camera.transform.position.z << " \n";
            }

            renderer->mainThreadDraw(&theScene.players[theScene.myPlayerIndex]->camera, gltfShader.shaderID, world.worldGenMethod, deltaTime);
            glUniform1f(timeRenderedLoc, 10.0f);
            glUniform1f(scaleLoc, 0.4f);
            for (auto& player : theScene.players | std::views::values)
            {

                auto pos = player->camera.transform.position + (player->camera.transform.direction*0.5f) + (player->camera.transform.right * 0.5f);
                pos.y -= 0.5f;
                glUniform3f(posLoc, pos.x, pos.y, pos.z);
                glUniform3f(offsetLoc, -0.5f, -0.5f, -0.5f);
                drawHandledBlock(player->camera.transform.position, player->currentHeldBlock, gltfShader.shaderID, player->lastHeldBlock, player->handledBlockMeshInfo);
            }
            glUniform3f(offsetLoc, 0.0f, 0.0f, 0.0f);
            glUniform1f(scaleLoc, 1.0f);
            gScene->fetchResults(true);


            //std::cout << "Passing " << camera.transform.position.x << " " << camera.transform.position.y << " " << camera.transform.position.z << " \n";



            glBindTexture(GL_TEXTURE_2D, worldTex.id);



            for(auto & gizmo : theScene.gizmos)
            {
                gizmo->draw(&world, theScene.players[theScene.myPlayerIndex]);
            }










            glBindVertexArray(billboardVAO);
            glUseProgram(billboardInstShader.shaderID);
            auto loc1 = glGetUniformLocation(billboardInstShader.shaderID, "mvp");
            //std::cout << "mvp loc: " << std::to_string(loc1) << "\n";
            glUniformMatrix4fv(loc1, 1, GL_FALSE, glm::value_ptr(camera.mvp));
            auto loc2 = glGetUniformLocation(billboardInstShader.shaderID, "camPos");
            //std::cout << "campos loc: " << std::to_string(loc2) << "\n";

            glUniform3f(loc2, camera.transform.position.x, camera.transform.position.y, camera.transform.position.z);
            auto loc3 = glGetUniformLocation(billboardInstShader.shaderID, "time");
            //std::cout << "time loc: " << std::to_string(loc3) << "\n";
            glUniform1f(loc3, glfwGetTime());

            texture2DArray.bind(2);

            auto loc4 = glGetUniformLocation(billboardInstShader.shaderID, "ourTexture");
            //std::cout << "texture loc: " << std::to_string(loc4) << "\n";
            glUniform1i(loc4, 2);
            // // Check for OpenGL errors
            // GLenum err;
            // while ((err = glGetError()) != GL_NO_ERROR) {
            //     std::cerr << "OpenGL error: " << err << std::endl;
            // }
            //
            // // Print uniform locations
            // std::cout << "mvp location: " << loc1 << std::endl;
            // std::cout << "camPos location: " << loc2 << std::endl;
            // std::cout << "time location: " << loc3 << std::endl;
            // std::cout << "ourTexture location: " << loc4 << std::endl;
            //
            // // Print texture ID
            // std::cout << "Texture ID: " << texture2DArray.textureID << std::endl;
            //
            //
            //
            //
            //
            // std::cout << "players size: " << theScene.players.size() << " \n";
            if (!billboards.empty())
            {
                glDisable(GL_CULL_FACE);

                jl::drawBillboards(billboards.size());
                glEnable(GL_CULL_FACE);
            }












            particles->cleanUpOldParticles(deltaTime);

            theScene.hud->draw();


        }

        renderImGui();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }


    glfwDestroyWindow(window);

}