//
// Created by jack on 1/27/2025.
//




#include "BasicShader.h"
#include "Player.h"
#include "Shader.h"
#include "Texture.h"

#include "PrecompHeader.h"



#include "Client.h"
#include "FPSCounter.h"
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
#include "world/gizmos/BulkPlaceGizmo.h"
#include "IndexOptimization.h" // This is quackery I apologize
#include "OpenALStuff.h"

#include "ModelLoader.h"
#include "BasicGLTFShader.h"


#include "SunAndMoon.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "Planets.h"
//Tinygltf includes stb image
//#include <stb_image.h>



boost::asio::io_context io_context;
boost::asio::ip::tcp::socket tsocket(io_context);
boost::asio::ip::tcp::resolver resolver(io_context);

Scene theScene = {};

constexpr double J_PI = 3.1415926535897932384626433832;
constexpr double DEG_TO_RAD = J_PI / 180.0;

__inline float gaussian(float x, float peak, float radius) {
    float stdDev = radius / 3.0;
    float variance = stdDev * stdDev;

    float b = exp(-pow(x - peak, 2.0) / (2.0 * variance));

    float peakHeight = exp(-pow(peak - peak, 2.0) / (2.0 * variance));
    return b / peakHeight;
};

__inline float ambBrightFromTimeOfDay(float timeOfDay, float dayLength)
{
    return std::max(0.05f, std::min(1.0f, gaussian(timeOfDay, dayLength/1.75f, dayLength/2.0f) * 1.3f));
}

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

        if(button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if(scene->mouseCaptured)
            {
                scene->blockSelectGizmo->hitting = action;
                if (!action)
                {
                    scene->blockSelectGizmo->hitProgress = 0.0f;
                }
            }
            if (action)
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


                }
            }



        }

    if(action)
    {

if(button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            auto & cam = scene->players[scene->myPlayerIndex]->camera;


            if (scene->multiplayer)
            {

                if (!scene->bulkPlaceGizmo->active && !scene->vmStampGizmo->active)
                {
                    if (scene->blockSelectGizmo->isDrawing)
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
                    }
                } else if (scene->bulkPlaceGizmo->active)
                {

                        auto playerpos = scene->players[scene->myPlayerIndex]->camera.transform.position;

                        bool isInside =
                        playerpos.x >= std::min(scene->bulkPlaceGizmo->corner1.x, scene->bulkPlaceGizmo->corner2.x) &&
                        playerpos.x <= std::max(scene->bulkPlaceGizmo->corner1.x, scene->bulkPlaceGizmo->corner2.x) &&
                        playerpos.y >= std::min(scene->bulkPlaceGizmo->corner1.y, scene->bulkPlaceGizmo->corner2.y) &&
                        playerpos.y <= std::max(scene->bulkPlaceGizmo->corner1.y, scene->bulkPlaceGizmo->corner2.y) &&
                        playerpos.z >= std::min(scene->bulkPlaceGizmo->corner1.z, scene->bulkPlaceGizmo->corner2.z) &&
                        playerpos.z <= std::max(scene->bulkPlaceGizmo->corner1.z, scene->bulkPlaceGizmo->corner2.z);

                        if(!isInside)
                        {
                            if (scene->bulkPlaceGizmo->placeMode == BulkPlaceGizmo::Solid)
                            {
                                DGMessage bulkPlace = BulkBlockSet{
                                    .corner1 = scene->bulkPlaceGizmo->corner1, .corner2 = scene->bulkPlaceGizmo->corner2, .block = scene->players.at(scene->myPlayerIndex)->currentHeldBlock, .hollow = false};
                                pushToMainToNetworkQueue(bulkPlace);
                                scene->bulkPlaceGizmo->active = false;
                            } else
                            {
                                IntTup minCorner = {
                                    std::min(scene->bulkPlaceGizmo->corner1.x, scene->bulkPlaceGizmo->corner2.x),
                                    std::min(scene->bulkPlaceGizmo->corner1.y, scene->bulkPlaceGizmo->corner2.y),
                                    std::min(scene->bulkPlaceGizmo->corner1.z, scene->bulkPlaceGizmo->corner2.z)
                                };

                                IntTup maxCorner = {
                                    std::max(scene->bulkPlaceGizmo->corner1.x, scene->bulkPlaceGizmo->corner2.x),
                                    std::max(scene->bulkPlaceGizmo->corner1.y, scene->bulkPlaceGizmo->corner2.y),
                                    std::max(scene->bulkPlaceGizmo->corner1.z, scene->bulkPlaceGizmo->corner2.z)
                                };

                                DGMessage bulkPlace = BulkBlockSet{
                                    .corner1 = minCorner, .corner2 = maxCorner, .block = scene->players.at(scene->myPlayerIndex)->currentHeldBlock, .hollow = true};
                                pushToMainToNetworkQueue(bulkPlace);
                                // DGMessage bulkCutout = BulkBlockSet{
                                //     minCorner + IntTup(1,1,1), maxCorner + IntTup(-1,-1,-1), AIR};
                                // pushToMainToNetworkQueue(bulkCutout);
                                scene->bulkPlaceGizmo->active = false;
                            }

                        }



                } else if (scene->vmStampGizmo->active)
                {
                    DGMessage vmplace = VoxModelStamp{
                        .name = (VoxelModelName)scene->vmStampGizmo->modelIndex,
                        .spot = scene->vmStampGizmo->spot
                       };
                    pushToMainToNetworkQueue(vmplace);
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
                    BlockType blockThere = scene->world->get(spot);
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

        if (key == GLFW_KEY_E && action == GLFW_PRESS)
        {
            if (currentGuiScreen == GuiScreen::InGame)
            {
                currentGuiScreen = GuiScreen::Inventory;
                uncaptureMouse(scene);
            } else if (currentGuiScreen == GuiScreen::Inventory)
            {
                currentGuiScreen = GuiScreen::InGame;
                captureMouse(scene);
            }
        }

        if (key == GLFW_KEY_V && action == GLFW_PRESS)
        {
            theScene.vmStampGizmo->active = !theScene.vmStampGizmo->active;
        }
        if (key == GLFW_KEY_F3)
        {
            if (action == GLFW_PRESS)
            {
                scene->showingControls = !scene->showingControls;
            }


        }

        if (key == GLFW_KEY_B)
        {
            if (action == GLFW_PRESS)
            {

                if (!scene->bulkPlaceGizmo->active)
                {
                    scene->bulkPlaceGizmo->corner1 = scene->blockSelectGizmo->selectedSpot;
                    scene->bulkPlaceGizmo->active = true;
                    scene->bulkPlaceGizmo->placeMode = BulkPlaceGizmo::Solid;
                } else if (scene->bulkPlaceGizmo->placeMode == BulkPlaceGizmo::Solid)
                {
                    scene->bulkPlaceGizmo->placeMode = BulkPlaceGizmo::Walls;
                } else
                {
                    scene->bulkPlaceGizmo->active = false;
                }





            }

        } else
        if(key == GLFW_KEY_ESCAPE && scene->mouseCaptured == true)
        {
            //glfwSetWindowShouldClose(window, 1);
            currentGuiScreen = GuiScreen::EscapeMenu;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            scene->mouseCaptured = false;
            scene->firstMouse = true;
        } else

            if (key == GLFW_KEY_T)
            {
                std::cout << "Num threads running: " << NUM_THREADS_RUNNING << '\n';
            } else
        if (key == GLFW_KEY_H)
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
    if(scene->mouseCaptured && scene->worldIntroTimer > 10.0f)
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

            camera.updateYPIndirect(camera.targetYaw + static_cast<float>(xOffset), camera.targetPitch + static_cast<float>(yOffset));
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
        scene->guiCamera->updateProjection(width, height, 60.0f);
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
    if (glfwGetTime() - lastTime > 0.005f)
    {
        lastTime = glfwGetTime();
        Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
        if (scene->myPlayerIndex != -1)
        {

            if (!scene->vmStampGizmo->active)
            {
                if (yoffset > 0)
                {
                    scene->players.at(scene->myPlayerIndex)->currentHeldBlock = (MaterialName)(((int)scene->players.at(scene->myPlayerIndex)->currentHeldBlock + 1 ) % BLOCK_COUNT);
                }
                if (yoffset < 0)
                {
                    int newMat = (scene->players.at(scene->myPlayerIndex)->currentHeldBlock - 1);
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
            } else
            {
                if (yoffset > 0)
                {
                   scene->vmStampGizmo->modelIndex = (scene->vmStampGizmo->modelIndex + 1) % voxelModels.size();
                }
                if (yoffset < 0)
                {
                    int newMat = (scene->vmStampGizmo->modelIndex - 1);
                    if (newMat < 0)
                    {
                        newMat = voxelModels.size() - 1;
                    }
                    scene->vmStampGizmo->modelIndex = newMat;
                }
            }

        }
    }

}



void enterWorld(Scene* s)
{
    s->worldIntroTimer = 0.0f;

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


    glfwWindowHint(GLFW_SAMPLES, 32); // Set MSAA samples (4x MSAA)


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
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glfwSwapInterval(0);
    glEnable(GL_MULTISAMPLE); // Enable MSAA in OpenGL

    initOpenAL();

    WorldRenderer* renderer = new WorldRenderer();
    theScene.loadSettings();

    renderer->currentRenderDistance = theScene.rendDistSelection;
    renderer->MIN_DISTANCE = renderer->currentRenderDistance + 1;

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

    static jl::Shader mainShader = getBasicShader();
    static jl::Shader gltfShader = getBasicGLTFShader();
    static jl::Texture worldTex("resources/world.png");




    ParticlesGizmo* particles = new ParticlesGizmo();
    theScene.particles = particles;
    theScene.gizmos.push_back(particles);

    BlockSelectGizmo* bsg = new BlockSelectGizmo();
    theScene.blockSelectGizmo = bsg;
    theScene.gizmos.push_back(bsg);




    BulkPlaceGizmo* bpg = new BulkPlaceGizmo();
    theScene.bulkPlaceGizmo = bpg;
    theScene.gizmos.push_back(bpg);

    theScene.bulkPlaceGizmo->mainGameShader = mainShader.shaderID;
    theScene.bulkPlaceGizmo->mainGameTexture = worldTex.id;

    VoxModelStampGizmo* vms = new VoxModelStampGizmo();
    vms->shaderProgram = mainShader.shaderID;
    theScene.vmStampGizmo = vms;

    //theScene.gizmos.push_back(vms);

    for(auto & gizmo : theScene.gizmos)
    {
        gizmo->init();
    }

    World world(
        new HashMapDataMap(),
        new OverworldWorldGenMethod(),
        new HashMapDataMap());

    theScene.world = &world;
    // VoxModel swcModel = loadSwc("resources/swctest.txt");
    // stampVoxelModelInWorld(&world,swcModel);
    //
    // stampPyramidInWorld(&world, 500, 250, 250);

    theScene.hud = new Hud();
    theScene.hud->rebuildDisplayData();



    theScene.worldRenderer = renderer;

    //theScene.addPlayerWithIndex(99);

    jl::prepareBillboard();

    std::vector<std::string> paths = {"resources/sprite/idle.png", "resources/sprite/run.png", "resources/sprite/jump.png", "resources/sprite/leftstrafe.png", "resources/sprite/rightstrafe.png", "resources/sprite/backwardsrun.png"};
    jl::Texture2DArray texture2DArray(paths);
    texture2DArray.bind(2);

    static jl::Shader billboardInstShader = getBillboardInstanceShader();

    theScene.guiCamera = new jl::Camera();
    std::cout << "width:" << width << " height:" << height << std::endl;
    theScene.guiCamera->updateProjection(width, height, 60.0f);
    theScene.guiCamera->updateWithYawPitch(0.0,0.0);
    //
    // static jl::ModelAndTextures clouds = jl::ModelLoader::loadModel("resources/models/clouds.glb", false);
    static jl::ModelAndTextures jp = jl::ModelLoader::loadModel("resources/models/jetpack.glb", false);
    static jl::ModelAndTextures jplit = jl::ModelLoader::loadModel("resources/models/jetpack2.glb", false);
    static jl::ModelAndTextures planet = jl::ModelLoader::loadModel("resources/models/planet.glb", false);


    while (!glfwWindowShouldClose(window))
    {
        static float lastTime = glfwGetTime();
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        int wwi, whei = 0;
        glfwGetWindowSize(window, &wwi, &whei);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        glBindVertexArray(billboardVAO);
        glUseProgram(billboardInstShader.shaderID);


        theScene.vmStampGizmo->spot = theScene.blockSelectGizmo->selectedSpot;




        if (theScene.myPlayerIndex != -1)
        {



            theScene.worldIntroTimer += deltaTime;
            updateFPS();
            theScene.timeOfDay = std::fmod(theScene.timeOfDay + deltaTime, theScene.dayLength);

            if (theScene.blockSelectGizmo->hitting)
            {
                if (theScene.blockSelectGizmo->hitProgress < 0.25f)
                {
                    theScene.blockSelectGizmo->hitProgress += deltaTime;
                } else
                {
                    theScene.blockSelectGizmo->hitProgress = 0.0f;

                    if (theScene.blockSelectGizmo)
                    {
                        if (theScene.blockSelectGizmo->isDrawing)
                        {
                            auto & cam = theScene.players[theScene.myPlayerIndex]->camera;



                            if (theScene.multiplayer)
                            {
                                auto & spot = theScene.blockSelectGizmo->selectedSpot;
                                //std::cout << "Senfing blokc chagne \n";
                                pushToMainToNetworkQueue(BlockSet{
                                    spot, AIR
                                });

                            } else
                            {
                                if (theScene.world && theScene.blockSelectGizmo && theScene.worldRenderer)
                                {
                                    //std::cout << "Setting" << std::endl;
                                    auto & spot = theScene.blockSelectGizmo->selectedSpot;
                                    //std::cout << "At Spot: " << spot.x << ", " << spot.y << ", " << spot.z << std::endl;
                                    BlockType blockThere = theScene.world->get(spot);
                                    glm::vec3 burstspot = glm::vec3(
                                        theScene.blockSelectGizmo->selectedSpot.x+ 0.5,
                                        theScene.blockSelectGizmo->selectedSpot.y + 0.5,
                                        theScene.blockSelectGizmo->selectedSpot.z + 0.5);
                                    if (theScene.particles)
                                    {
                                        theScene.particles->particleBurst(burstspot,
                                                                     12, (MaterialName)blockThere, 2.0, 1.0f);
                                    }

                                    //theScene.world->set(spot, AIR);
                    //std::cout << "Set the block "  << std::endl;;
                                    auto cs = WorldRenderer::chunkSize;
                                    // Then in your code:
                                    auto xmod = properMod(spot.x, cs);
                                    auto zmod = properMod(spot.z, cs);
                                    //std::cout << "Xmod: " << xmod << " Zmod: " << zmod << std::endl;
                                    theScene.worldRenderer->requestChunkRebuildFromMainThread(
                                        spot, AIR, false
                                        );

                                    if(xmod == WorldRenderer::chunkSize - 1)
                                    {
                                        theScene.worldRenderer->requestChunkRebuildFromMainThread(
                                            IntTup(spot.x+1, spot.y, spot.z));

                                    } else if(xmod == 0)
                                    {
                                        theScene.worldRenderer->requestChunkRebuildFromMainThread(
                                            IntTup(spot.x-1, spot.y, spot.z));
                                    }

                                    if(zmod == WorldRenderer::chunkSize - 1)
                                    {
                                        theScene.worldRenderer->requestChunkRebuildFromMainThread(
                                            IntTup(spot.x, spot.y, spot.z+1));

                                    } else if(zmod == 0)
                                    {
                                        theScene.worldRenderer->requestChunkRebuildFromMainThread(
                                            IntTup(spot.x, spot.y, spot.z-1));
                                    }
                                    theScene.worldRenderer->requestChunkRebuildFromMainThread(
                                        spot
                                        );

                                    //std::cout << "Request filed " << std::endl;
                                }
                            }
                        }
                    }

                }
            }

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




            player->collisionCage.updateToSpot(&world, player->camera.transform.position, deltaTime);
            player->camera.updateWithYawPitch(player->camera.transform.yaw, player->camera.transform.pitch);
            if (id == theScene.myPlayerIndex)
            {
                player->camera.interpTowardTargetYP(deltaTime);
            } else
            {
                player->camera.interpTowardTargetYP(deltaTime*0.35f);
            }

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
                static_cast<float>(glfwGetTime()),
                static_cast<AnimationName>(wantedAnim) == JUMP ? 0.5f : 1.0f
            };

            bool jumpinprogress = (static_cast<AnimationName>(existinganim->actionNum) == JUMP) && (glfwGetTime() - existinganim->timestarted < 0.75f);
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
                        theScene.players.at(m.myPlayerIndex)->camera.targetYaw = m.startYawPitch.x;
                        theScene.players.at(m.myPlayerIndex)->camera.targetPitch = m.startYawPitch.y;
                    }
                    else if constexpr (std::is_same_v<T, YawPitchUpdate>)
                    {
                        if(!theScene.players.contains(m.myPlayerIndex))
                        {
                            theScene.addPlayerWithIndex(m.myPlayerIndex);
                        }
                        theScene.players.at(m.myPlayerIndex)->camera.updateYPIndirect(m.newYaw, m.newPitch);

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
                    else if constexpr (std::is_same_v<T, RequestInventorySwap>)
                    {
                        InventorySlot* source = nullptr;
                        InventorySlot* destination = nullptr;

                        if (m.mouseSlotS)
                        {
                            source = &(theScene.players.at(m.myPlayerIndex)->inventory.mouseHeldItem);
                        } else
                        {
                            source = &(theScene.players.at(m.myPlayerIndex)->inventory.inventory.at(m.sourceIndex));
                        }

                        if (m.mouseSlotD)
                        {
                            destination = &(theScene.players.at(m.myPlayerIndex)->inventory.mouseHeldItem);
                        } else
                        {
                            destination = &(theScene.players.at(m.myPlayerIndex)->inventory.inventory.at(m.destinationIndex));
                        }

                        if (source && destination)
                        {
                            InventorySlot srcCopy = *source;
                            *source = *destination;
                            *destination = srcCopy;
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
                        if (theScene.existingInvs.contains(m.id))
                        {
                            if (auto inv = loadInvFromFile("mpworld.txt", m.id))
                            {
                                std::cout << "Loaded inv to player " << m.index << " with id " << m.id << "\n";
                                theScene.players.at(m.index)->inventory = inv.value();
                            }
                        } else
                        {
                            theScene.players.at(m.index)->inventory.inventory = DEFAULT_INVENTORY;
                        }
                        theScene.players.at(m.index)->camera.transform.position = m.position;
                        theScene.players.at(m.index)->camera.transform.direction = m.direction;
                    }
                    else if constexpr (std::is_same_v<T, BlockSet>) {
                        //std::cout << "Processing network block change \n";

                        Scene* scene = &theScene;
                        //std::cout << " Server Setting" << std::endl;
                            auto & spot = m.spot;
                            //std::cout << "At Spot: " << spot.x << ", " << spot.y << ", " << spot.z << std::endl;
                            BlockType blockThere = scene->world->get(spot);
                            glm::vec3 burstspot = glm::vec3(
                                spot.x+ 0.5,
                                spot.y + 0.5,
                                spot.z + 0.5);
                            if (scene->particles)
                            {
                                scene->particles->particleBurst(burstspot,
                                                             12, (MaterialName)blockThere, 0.8, 0.0f);
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
                            else if constexpr (std::is_same_v<T, BulkBlockSet>) {
                                theScene.worldRenderer->requestBlockBulkPlaceFromMainThread(
                                    BlockArea{.corner1 = m.corner1, .corner2 = m.corner2, .block = m.block, .hollow = m.hollow}
                                    );
                                // if (m.hollow)
                                // {
                                //     theScene.worldRenderer->requestBlockBulkPlaceFromMainThread(
                                //     BlockArea{.corner1 = m.corner1 + IntTup(1,1,1), .corner2 = m.corner2 + IntTup(-1,-1,-1), .block = AIR, .hollow = false}
                                //     );
                                // }
                            }
                             else if constexpr (std::is_same_v<T, VoxModelStamp>) {
                                theScene.worldRenderer->requestVoxelModelPlaceFromMainThread(
                                    PlacedVoxModel{
                                        m.name, m.spot
                                    }
                                    );
                                // if (m.hollow)
                                // {
                                //     theScene.worldRenderer->requestBlockBulkPlaceFromMainThread(
                                //     BlockArea{.corner1 = m.corner1 + IntTup(1,1,1), .corner2 = m.corner2 + IntTup(-1,-1,-1), .block = AIR, .hollow = false}
                                //     );
                                // }
                            }
                        }, msg);



                break; //Only do one per frame
            }





            gScene->simulate(deltaTime);


            auto & camera = theScene.players.at(theScene.myPlayerIndex)->camera;




            setListenerToCameraPos(&camera);

            BlockArea m = {};

            while (theScene.worldRenderer->rebuildToMainAreaNotifications.pop(&m))
            {
                int minX = std::min(m.corner1.x, m.corner2.x);
                int maxX = std::max(m.corner1.x, m.corner2.x);
                int minZ = std::min(m.corner1.z, m.corner2.z);
                int maxZ = std::max(m.corner1.z, m.corner2.z);

                std::unordered_set<TwoIntTup, TwoIntTupHash> implicated = {};

                for (int x = minX; x <= maxX; x++) {
                        for (int z = minZ; z <= maxZ; z++) {
                            TwoIntTup cposhere = theScene.worldRenderer->worldToChunkPos(TwoIntTup(x, z));
                            if (theScene.worldRenderer->activeChunks.contains(cposhere))
                            {
                                implicated.insert(cposhere);

                            }
                        }
                }
                for (auto& i : implicated)
                {
                    theScene.worldRenderer->requestChunkSpotRebuildFromMainThread(i);
                }

                break; //Only do oen per frame
            }

            Atmosphere currAtmos = skyAndFogColor(theScene.currentPlanetType);

            if (theScene.bulkPlaceGizmo->active)
            {
                auto thespot = camera.transform.position + (camera.transform.direction * 3.0f);
                auto theintspot = IntTup(thespot.x, thespot.y, thespot.z);
                theScene.bulkPlaceGizmo->corner2 = theintspot;
            }
            if (theScene.worldIntroTimer > 10.0f)
            {
                drawSky(glm::vec4(currAtmos.skyTop, 1.0),
                    glm::vec4(currAtmos.skyBottom, 1.0),
                    ambBrightFromTimeOfDay(theScene.timeOfDay, theScene.dayLength), &theScene.players[theScene.myPlayerIndex]->camera, lutTexture);

            }


            glUseProgram(mainShader.shaderID);

            static GLuint mvpLoc = glGetUniformLocation(mainShader.shaderID, "mvp");
            static GLuint texture1Loc = glGetUniformLocation(mainShader.shaderID, "texture1");
            static GLuint lutLoc = glGetUniformLocation(mainShader.shaderID, "lut");
            static GLuint posLoc = glGetUniformLocation(mainShader.shaderID, "pos");
            static GLuint rotLoc = glGetUniformLocation(mainShader.shaderID, "rot");
            static GLuint camPosLoc = glGetUniformLocation(mainShader.shaderID, "camPos");
            static GLuint grcLoc = glGetUniformLocation(mainShader.shaderID, "grassRedChange");
            static GLuint scaleLoc = glGetUniformLocation(mainShader.shaderID, "scale");
            static GLuint timeRenderedLoc = glGetUniformLocation(mainShader.shaderID, "timeRendered");
            static GLuint offsetLoc = glGetUniformLocation(mainShader.shaderID, "offs");
            static GLuint abLoc = glGetUniformLocation(mainShader.shaderID, "ambientBrightness");
            static GLuint fogColLoc = glGetUniformLocation(mainShader.shaderID, "fogCol");

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(camera.mvp));
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(texture1Loc, 0);
            glUniform1f(abLoc, ambBrightFromTimeOfDay(theScene.timeOfDay, theScene.dayLength));
            glUniform3f(posLoc, 0.0, 0.0, 0.0);
            const glm::vec3 campos = theScene.players[theScene.myPlayerIndex]->camera.transform.position;
            glUniform3f(camPosLoc, campos.x, campos.y, campos.z);
            glUniform1f(rotLoc, 0.0f);
            glUniform3f(offsetLoc, 0.0f, 0.0f, 0.0f);
            glUniform1f(scaleLoc, 1.0f);

            glUniform3f(fogColLoc, currAtmos.fogColor.x, currAtmos.fogColor.y, currAtmos.fogColor.z);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_3D, lutTexture);
            glUniform1i(lutLoc, 1);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, worldTex.id);



            renderer->mainThreadDraw(&theScene.players[theScene.myPlayerIndex]->camera, mainShader.shaderID, world.worldGenMethod, deltaTime, theScene.worldIntroTimer > 10.0f);
            vms->draw(&world, theScene.players.at(theScene.myPlayerIndex));

            glUniform1f(timeRenderedLoc, 10.0f);
            glUniform1f(scaleLoc, 0.4f);
            for (auto& [id, player] : theScene.players)
            {
                auto equippeditems = player->inventory.getEquippedItems();
                bool jpEquipped = false;
                for (auto& equippeditem : equippeditems)
                {
                    if (equippeditem.block == ItemName::JETPACK)
                    {
                        jpEquipped = true;
                    }
                }
                auto pos = player->camera.transform.position + (player->camera.transform.direction*0.5f) + (player->camera.transform.right * 0.5f);
                pos.y -= 0.5f;

                glUseProgram(mainShader.shaderID);

                glUniform3f(offsetLoc, -0.5f, -0.5f, -0.5f);
                bool isme = id == theScene.myPlayerIndex;
                if (isme)
                {
                    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(theScene.guiCamera->mvp));
                    glUniform3f(posLoc, 3.0, -1.2, std::min((float)wwi / 600.0f, 2.0f));
                    glDisable(GL_DEPTH_TEST);
                    glUniform3f(camPosLoc, 0, 0, 0);
                } else
                {
                    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(camera.mvp));
                    glUniform3f(posLoc, pos.x, pos.y, pos.z);

                }

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, worldTex.id);


                drawHandledBlock(player->camera.transform.position, player->currentHeldBlock, mainShader.shaderID, player->lastHeldBlock, player->handledBlockMeshInfo);
                glEnable(GL_DEPTH_TEST);

                if (jpEquipped)
                {
                    {
                        glUseProgram(gltfShader.shaderID);

                        glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
                        glActiveTexture(GL_TEXTURE0);

                        if (player->controls.secondary1)
                        {
                            glBindTexture(GL_TEXTURE_2D, jplit.texids.at(0));
                        }
                        else
                        {
                            glBindTexture(GL_TEXTURE_2D, jp.texids.at(0));
                        }

                        glUniform1i(glGetUniformLocation(gltfShader.shaderID, "texture1"), 0);

                        auto camdir = player->camera.transform.direction;
                        camdir.y = 0;
                        camdir = glm::normalize(camdir);

                        glm::vec3 posToRenderAt = player->camera.transform.position - (camdir * 0.3f);
                        posToRenderAt.y = player->camera.transform.position.y - 0.5f;
                        glUniform3f(glGetUniformLocation(gltfShader.shaderID, "pos"), posToRenderAt.x, posToRenderAt.y, posToRenderAt.z);
                        glUniform1f(glGetUniformLocation(gltfShader.shaderID, "rot"), player->camera.transform.yaw);

                        for(jl::ModelGLObjects &mglo : jp.modelGLObjects)
                        {
                            glBindVertexArray(mglo.vao);
                            //Indent operations on this vertex array object
                            glDrawElements(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr);

                            glBindVertexArray(0);
                        }
                    }
                }

            }
            glUniform3f(offsetLoc, 0.0f, 0.0f, 0.0f);
            glUniform1f(scaleLoc, 1.0f);

            // {
            //     glUseProgram(gltfShader.shaderID);
            //
            //     glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
            //     glActiveTexture(GL_TEXTURE0);
            //     glBindTexture(GL_TEXTURE_2D, clouds.texids.at(0));
            //     glUniform1i(glGetUniformLocation(gltfShader.shaderID, "texture1"), 0);
            //     glUniform3f(glGetUniformLocation(gltfShader.shaderID, "pos"), 0.0f, 170.0f, 0.0f + (glfwGetTime()* 0.3));
            //     glUniform1f(glGetUniformLocation(gltfShader.shaderID, "rot"), 0.0f);
            //
            //     for(jl::ModelGLObjects &mglo : clouds.modelGLObjects)
            //     {
            //         glBindVertexArray(mglo.vao);
            //         //Indent operations on this vertex array object
            //         glDrawElements(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr);
            //
            //         glBindVertexArray(0);
            //     }
            // }

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

            if (theScene.worldIntroTimer < 10.0f)
            {

                drawFullscreenKaleidoscope();
                glClear(GL_DEPTH_BUFFER_BIT);
                {
                    glUseProgram(gltfShader.shaderID);

                    glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
                    glActiveTexture(GL_TEXTURE0);

                        glBindTexture(GL_TEXTURE_2D, planet.texids.at(0));


                    glUniform1i(glGetUniformLocation(gltfShader.shaderID, "texture1"), 0);

                    auto camdir = camera.transform.direction;

                    glm::vec3 posToRenderAt = camera.transform.position + (camdir * (100.0f - 100.0f *(theScene.worldIntroTimer / 11.3f)));

                    glUniform3f(glGetUniformLocation(gltfShader.shaderID, "pos"), posToRenderAt.x, posToRenderAt.y, posToRenderAt.z);
                    glUniform1f(glGetUniformLocation(gltfShader.shaderID, "rot"), camera.transform.yaw);

                    for(jl::ModelGLObjects &mglo : planet.modelGLObjects)
                    {
                        glBindVertexArray(mglo.vao);
                        //Indent operations on this vertex array object
                        glDrawElements(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr);

                        glBindVertexArray(0);
                    }
                }
            }


            drawSunAndMoon(&camera, &theScene);









            particles->cleanUpOldParticles(deltaTime);

            theScene.hud->draw();


        }

        renderImGui();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }


    glfwDestroyWindow(window);

}