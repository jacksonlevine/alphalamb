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

#include "DrawSky.h"
#include "FileArchives.h"
#include "LightSpeedTest.h"
#include "Planets.h"
#include "SaveRegistry.h"
#include "StepTimerProfiler.h"
#include "specialblocks/FindSpecialBlock.h"
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

    //auto& player = theScene.players.at(theScene.myPlayerIndex);

    static Controls lastcontrols = theScene.our<Controls>();

    auto ourcontrols = theScene.our<Controls>();
    if(ourcontrols != lastcontrols)
    {
        lastcontrols = ourcontrols;
        auto ourt = theScene.our<jl::Camera>().transform;
        DGMessage cu = ControlsUpdate(theScene.myPlayerIndex, ourcontrols, ourt.position, glm::vec2(ourt.yaw, ourt.pitch));
        pushToMainToNetworkQueue(cu);

    }

    if (timer > 0.2)
    {
        timer = 0.0f;

        auto ourt = theScene.our<jl::Camera>().transform;

        static float lastpitch = ourt.pitch;
        static float lastyaw = ourt.yaw;

        if(lastpitch != ourt.pitch || lastyaw != ourt.yaw)
        {
            lastpitch = ourt.pitch;
            lastyaw = ourt.yaw;

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
            auto & cam = theScene.our<jl::Camera>();

                if (scene->multiplayer)
                {

                    if (!scene->bulkPlaceGizmo->active && !scene->vmStampGizmo->active)
                    {
                        if (scene->blockSelectGizmo->isDrawing)
                        {

                            auto & spot = scene->blockSelectGizmo->selectedSpot;
                            auto hn = scene->blockSelectGizmo->hitNormal;

                            glm::vec3 spotf(spot.x + 0.5f, spot.y + 0.5f, spot.z + 0.5f);
                            glm::vec3 placespotf = spotf + glm::vec3(hn.x, hn.y, hn.z);

                            IntTup placeSpot(std::floor(placespotf.x), std::floor(placespotf.y), std::floor(placespotf.z));

                            using namespace std;


                            BlockType blockThere = scene->world->get(spot);
                            if (blockThere == DOOR)
                            {
                                pushToMainToNetworkQueue(BlockSet{
                                        .spot = spot, .block =  DOOR, .pp = cam.transform.position
                                    });
                            } else
                            {
                                IntTup playerBlockSpot1 = IntTup(floor(cam.transform.position.x), floor(cam.transform.position.y), floor(cam.transform.position.z));
                                IntTup playerBlockSpot2 = IntTup(floor(cam.transform.position.x), floor(cam.transform.position.y-1), floor(cam.transform.position.z));

                                if (placeSpot != playerBlockSpot1 && placeSpot != playerBlockSpot2)
                                {
                                    //std::cout << "Senfing blokc place \n";

                                    pushToMainToNetworkQueue(BlockSet{
                                        .spot = placeSpot, .block =  scene->our<InventoryComponent>().currentHeldBlock, .pp = cam.transform.position
                                    });
                                }
                            }

                        }
                    } else if (scene->bulkPlaceGizmo->active)
                    {

                            auto playerpos = scene->our<jl::Camera>().transform.position;

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
                                        .corner1 = scene->bulkPlaceGizmo->corner1, .corner2 = scene->bulkPlaceGizmo->corner2, .block = scene->our<InventoryComponent>().currentHeldBlock, .hollow = false};
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
                                        .corner1 = minCorner, .corner2 = maxCorner, .block = scene->our<InventoryComponent>().currentHeldBlock, .hollow = true};
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

                }




        } else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            if (scene->world && scene->blockSelectGizmo && scene->worldRenderer && scene->myPlayerIndex != entt::null)
            {
                if (scene->blockSelectGizmo->isDrawing)
                {
                    //std::cout << "Setting" << std::endl;
                    auto & spot = scene->blockSelectGizmo->selectedSpot;
                    //std::cout << "At Spot: " << spot.x << ", " << spot.y << ", " << spot.z << std::endl;
                    BlockType blockThere = scene->world->get(spot);
                    //IntTup placeSpot = scene->blockSelectGizmo->selectedSpot + scene->blockSelectGizmo->hitNormal;
                    scene->our<InventoryComponent>().currentHeldBlock = (MaterialName)blockThere;
                    //scene->world->set(spot, AIR);
                    //std::cout << "Set the block "  << std::endl;;
                    // scene->worldRenderer->requestChunkRebuildFromMainThread(
                    //     placeSpot, scene->players.at(scene->myPlayerIndex)->currentHeldBlock
                    //     );


                } else
                {
                    scene->our<InventoryComponent>().currentHeldBlock = AIR;
                }

                //std::cout << "Request filed " << std::endl;
            }
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    // if(GLFW_KEY_T == key && GLFW_PRESS == action)
    // {
    //     ALint jpSourceState;
    //     alGetSourcei(theScene.musicSource, AL_SOURCE_STATE, &jpSourceState);
    //     alSourcePlay(theScene.musicSource);
    //     std::cout << "music source state: " << jpSourceState << std::endl;
    // }
    if(GLFW_KEY_1 == key && GLFW_PRESS == action)
    {
        const bool b = entt::monostate<entt::hashed_string{"activeHand"}>{};
        entt::monostate<entt::hashed_string{"activeHand"}>{} = !b;
    }

    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));

    if (!scene->mouseCaptured)
    {
        if (imguiio->WantCaptureKeyboard) return;
    }
    if (scene->myPlayerIndex != entt::null)
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

        if (key == GLFW_KEY_G && action == GLFW_PRESS)
        {
            if ((scene->lastBlockAtCursor & BLOCK_ID_BITS) == DG_COMPUTERBLOCK)
            {

                if (currentGuiScreen == GuiScreen::InGame)
                {
                    auto spot = scene->blockSelectGizmo->selectedSpot;
                    auto view = scene->REG.view<ComputerComponent, NPPositionComponent>();
                    for (auto entity : view)
                    {
                        auto & comp = view.get<ComputerComponent>(entity);
                        auto & np = view.get<NPPositionComponent>(entity);

                        auto pos = np.position;
                        if (pos == glm::vec3(spot.x, spot.y, spot.z))
                        {
                            scene->currentEditor = &comp.editor;
                            break;
                        }
                    }

                    currentGuiScreen = GuiScreen::Computer;
                    uncaptureMouse(scene);
                } else if (currentGuiScreen == GuiScreen::Computer)
                {
                    scene->currentEditor = nullptr;
                    currentGuiScreen = GuiScreen::InGame;
                    captureMouse(scene);
                }
            }
        }

        if (key == GLFW_KEY_L && action == GLFW_PRESS)
        {
            saveRegistry(scene->REG, "snapshot.bin");
        }

        if (key == GLFW_KEY_LEFT_CONTROL)
        {
            scene->our<Controls>().crouch = action;
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

            auto & pos = scene->our<jl::Camera>().transform.position;
            //scene->worldRenderer->printMemoryFootprint();

        } else
        if (key == GLFW_KEY_H)
        {
            scene->our<Controls>().secondary2 = action;
        } else
        if (key == GLFW_KEY_LEFT_SHIFT)
        {
            scene->our<Controls>().sprint = action;
        } else
        if (key == GLFW_KEY_F)
        {

            scene->our<Controls>().secondary1 = action;
            scene->our<Controls>().jump = action;






        } else
        if (key == GLFW_KEY_W)
        {
            scene->our<Controls>().forward = action;
        } else
        if (key == GLFW_KEY_A)
        {
            scene->our<Controls>().left = action;
        } else
        if (key == GLFW_KEY_S)
        {
            scene->our<Controls>().backward = action;
        } else
        if (key == GLFW_KEY_D)
        {
            scene->our<Controls>().right = action;
        } else
        if (key == GLFW_KEY_SPACE)
        {
            scene->our<Controls>().jump = action;
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

        if (scene->myPlayerIndex != entt::null)
        {
            // print(cout, "xOffset: {}, yOffset: {} \n", xOffset, yOffset);
            auto & camera = scene->our<jl::Camera>();

            //camera.updateYPIndirect(camera.targetYaw + static_cast<float>(xOffset), camera.targetPitch + static_cast<float>(yOffset));

            camera.transform.yaw += static_cast<float>(xOffset);
            camera.transform.pitch += static_cast<float>(yOffset);

            //scene->guiCamera->updateWithYawPitch(scene->guiCamera->transform.yaw + static_cast<float>(xOffset), scene->guiCamera->transform.pitch + static_cast<float>(yOffset));
        }

        lastx = xpos;
        lasty = ypos;
    }
}

void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
    if (scene->myPlayerIndex != entt::null)
    {
        auto & camera = scene->our<jl::Camera>();
        camera.updateProjection(width, height, 90.0f);
        scene->guiCamera->updateProjection(width, height, 60.0f);
        scene->menuCamera->updateProjection(width, height, 60.0f);
    }

    entt::monostate<entt::hashed_string{"swidth"}>{} = width;
    entt::monostate<entt::hashed_string{"sheight"}>{} = height;


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
    if (glfwGetTime() - lastTime > 0.005f && currentGuiScreen == GuiScreen::InGame)
    {
        lastTime = glfwGetTime();
        Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
        if (scene->myPlayerIndex != entt::null)
        {

            if (!scene->vmStampGizmo->active)
            {
                if (yoffset > 0)
                {
                    scene->our<InventoryComponent>().currentHeldBlock = (MaterialName)(((int)scene->our<InventoryComponent>().currentHeldBlock + 1 ) % BLOCK_COUNT);
                }
                if (yoffset < 0)
                {
                    int newMat = (scene->our<InventoryComponent>().currentHeldBlock - 1);
                    if (newMat < 0)
                    {
                        newMat = BLOCK_COUNT - 1;
                    }
                    scene->our<InventoryComponent>().currentHeldBlock = (MaterialName)newMat;
                }

                if (scene->multiplayer)
                {
                    DGMessage sbu = PlayerSelectBlockChange{
                        scene->myPlayerIndex,
                    scene->our<InventoryComponent>().currentHeldBlock};
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

    // if (!s->multiplayer)
    // {
    //     //std::cout << " This happening \n";
    //     s->myPlayerIndex = s->addPlayer();
    // }
    //Add ourselves to the scene


    {

        auto & camera = s->our<jl::Camera>();
        camera.updateProjection(width, height, 90.0f);
    }

    s->worldRenderer->launchThreads(&s->our<jl::Camera>(), s->world);
}
// Define a regular function with the correct signature
static void onPhysicsComponentAdded(entt::registry& reg, entt::entity entity) {

}

const PxU32 scratchMemorySize = 16 * 1024;

void* scratchMemory = nullptr;

int main()
{

    scratchMemory = _aligned_malloc(scratchMemorySize, 16);

    StepTimerProfiler<false> profiler;

    glfwInit();
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);


   // glfwWindowHint(GLFW_SAMPLES, 32); // Set MSAA samples (4x MSAA)

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    theScene.window = glfwCreateWindow(800, 800, "project7", nullptr, nullptr);
    GLFWwindow* window = theScene.window;
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
    };


    theScene.REG.on_construct<PhysicsComponent>().connect<&onPhysicsComponentAdded>();
    entt::monostate<entt::hashed_string{"swidth"}>{} = 800;
    entt::monostate<entt::hashed_string{"sheight"}>{} = 800;

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
    //glEnable(GL_MULTISAMPLE); // Enable MSAA in OpenGL

    initOpenAL();

    theScene.musicSource = makeSource(glm::vec3{});

    WorldRenderer* renderer = new WorldRenderer();
    theScene.loadSettings();

    theScene.playSong(sounds.at((int)SoundBuffers::SONG1), true);
    
    renderer->currentRenderDistance = theScene.rendDistSelection;

    initializePhysX();

    loadGameVoxelModels();

    const GLubyte* version = glGetString(GL_SHADING_LANGUAGE_VERSION);
    if (version) {
        std::cout << "GLSL version: " << version << std::endl;
    }

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

    theScene.font_title = imguiio->Fonts->AddFontFromFileTTF("font.ttf", 40.0f, NULL, imguiio->Fonts->GetGlyphRangesDefault());


    BulkPlaceGizmo* bpg = new BulkPlaceGizmo();
    theScene.bulkPlaceGizmo = bpg;
    theScene.gizmos.push_back(bpg);

    theScene.bulkPlaceGizmo->mainGameShader = mainShader.shaderID;
    theScene.bulkPlaceGizmo->mainGameTexture = worldTex.id;

    VoxModelStampGizmo* vms = new VoxModelStampGizmo();
    vms->shaderProgram = mainShader.shaderID;
    theScene.vmStampGizmo = vms;

    //theScene.gizmos.push_back(vms);

    lighttest();

    for(auto & gizmo : theScene.gizmos)
    {
        gizmo->init();
    }

    World world(
        new HashMapDataMap(),
        new OverworldWorldGenMethod());

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
    //std::cout << "width:" << width << " height:" << height << std::endl;
    theScene.guiCamera->updateProjection(width, height, 60.0f);
    theScene.guiCamera->updateWithYawPitch(0.0,0.0);

    theScene.menuCamera = new jl::Camera();
    theScene.menuCamera->transform.position = glm::vec3(0.0, 1700.0f, 0.0f);
    theScene.menuCamera->updateProjection(width, height, 120.0f);
    theScene.menuCamera->updateWithYawPitch(0.0,-89.9);

    //
    // static jl::ModelAndTextures clouds = jl::ModelLoader::loadModel("resources/models/clouds.glb", false);
    static jl::ModelAndTextures jp = jl::ModelLoader::loadModel("resources/models/jetpack.glb", false);
    static jl::ModelAndTextures jplit = jl::ModelLoader::loadModel("resources/models/jetpack2.glb", false);
    static jl::ModelAndTextures planet = jl::ModelLoader::loadModel("resources/models/planet.glb", false);

    //SetImGuiScaling(window);

    entt::monostate<entt::hashed_string{"activeHand"}>{} = true;

    theScene.worldtex = worldTex.id;

    while (!glfwWindowShouldClose(window)) {

        {
            std::string temp;
            std::string output_str = theScene.pythonContext.g_output.str();
            std::istringstream stream(output_str);

            // Reset the ostringstream completely
            theScene.pythonContext.g_output.str("");
            theScene.pythonContext.g_output.clear();

            while(std::getline(stream, temp)) {
                theScene.messages.push_back(ChatMessage{
                    .text = temp,
                    .timestamp = std::chrono::steady_clock::now()
                });
            }
        }


        static float lastTime = glfwGetTime();
        float currentTime = glfwGetTime();
        deltaTime = std::min(0.03f, currentTime - lastTime);
        lastTime = currentTime;


        int wwi, whei = 0;
        glfwGetWindowSize(window, &wwi, &whei);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glBindVertexArray(billboardVAO);
        glUseProgram(billboardInstShader.shaderID);


        theScene.vmStampGizmo->spot = theScene.blockSelectGizmo->selectedSpot;


        profiler.checkTime("Set some crap");

        if (theScene.myPlayerIndex != entt::null && theScene.worldReceived.load())
        {
            // static int ccounnc = 0;
            // if(ccounnc > 100)
            // {
            //     ccounnc = 0;
            //     auto pos = theScene.our<jl::Camera>().transform.position;
            //     std::cout << "Posit " << pos.x << " " << pos.y << " " << pos.z << std::endl;
            // } else
            // {
            //     ccounnc += 1;
            // }









            if (auto t = theScene.world->tryToGetReadLockOnDMsOnly(); t != std::nullopt)
            {
                theScene.lastBlockAtCursor = theScene.world->getRawLocked(theScene.blockSelectGizmo->selectedSpot);

                auto pos = theScene.our<jl::Camera>().transform.position;
                auto & cont = theScene.our<Controls>();
                auto crouched = cont.crouch;
                theScene.blockHeadIn = (MaterialName)(theScene.world->getRawLocked(IntTup(pos.x, pos.y, pos.z)) & BLOCK_ID_BITS);
                if (theScene.our<MovementComponent>().crouchOverride)
                {
                    theScene.blockFeetIn = theScene.blockHeadIn;
                    theScene.blockFeetInLower = (MaterialName)(theScene.world->getRawLocked(IntTup(pos.x, std::floor(pos.y - 0.5f), pos.z)) & BLOCK_ID_BITS);
                } else
                {
                    theScene.blockFeetIn = (MaterialName)(theScene.world->getRawLocked(IntTup(pos.x, (pos.y - 1.0f), pos.z)) & BLOCK_ID_BITS);
                    theScene.blockFeetInLower = (MaterialName)(theScene.world->getRawLocked(IntTup(pos.x, std::floor(pos.y - 1.0f - 0.5f), pos.z)) & BLOCK_ID_BITS);
                }
                auto blockFeetInWater = liquids.test(theScene.blockFeetIn);
                auto blockFeetInLowerWater = liquids.test(theScene.blockFeetInLower);
                if(blockFeetInWater)
                {
                    cont.swimming = true;
                } else
                {
                    if(!blockFeetInLowerWater)
                    {
                        cont.swimming = false;
                    }
                }


            }
            profiler.checkTime("Update some blocks");


            // auto pos = theScene.getOur<jl::Camera>().transform.position;
            // std::cout << "Position: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
            //std::cout << "world received and stuff" << std::endl;



            updateFPS();
            theScene.timeOfDay = std::fmod(theScene.timeOfDay + deltaTime, theScene.dayLength);


            if (theScene.blockSelectGizmo->hitting)
            {
                auto ourpp = theScene.our<jl::Camera>().transform.position;
                if (theScene.blockSelectGizmo->hitProgress < BLOCK_BREAK_TIME)
                {
                    theScene.blockSelectGizmo->hitProgress += deltaTime;
                } else
                {
                    theScene.blockSelectGizmo->hitProgress = 0.0f;

                    if (theScene.blockSelectGizmo)
                    {
                        if (theScene.blockSelectGizmo->isDrawing)
                        {
                            auto & cam = theScene.our<jl::Camera>();



                            if (theScene.multiplayer)
                            {
                                auto & spot = theScene.blockSelectGizmo->selectedSpot;
                                //std::cout << "Senfing blokc chagne \n";
                                pushToMainToNetworkQueue(BlockSet{
                                   .spot = spot, .block = AIR, .pp = glm::vec3(0.f)
                                });

                            }
                        }
                    }

                }
            }
            profiler.checkTime("Update hit profress");
            std::vector<Billboard> billboards;
            std::vector<AnimationState> animStates;
            billboards.reserve(10);
            animStates.reserve(10);
            auto simscene = theScene.worldIntroTimer >= 1.0f;
            //std::cout << "World intro timer " << theScene.worldIntroTimer << std::endl;
            if(theScene.worldIntroTimer < 1.0f)
            {
                if(theScene.worldReceived.load())
                {
                    theScene.worldIntroTimer += deltaTime;
                }

            }

            auto playerView = theScene.REG.view<RenderComponent, PhysicsComponent, Controls, jl::Camera, MovementComponent, InventoryComponent, ParticleEffectComponent>();
            profiler.checkTime("Make some variables");
            if (simscene)
            {
                if(theScene.multiplayer)
                {
                    sendControlsUpdatesLol(tsocket, deltaTime);
                }
                profiler.checkTime("Send control updates");

                for (auto entity : playerView)
                {

                    auto & renderComponent = playerView.get<RenderComponent>(entity);
                    auto & movementComponent = playerView.get<MovementComponent>(entity);
                    auto & inventory = playerView.get<InventoryComponent>(entity);
                    auto & particleComponent = playerView.get<ParticleEffectComponent>(entity);
                    auto & billboard = renderComponent.billboard;

                    auto & camera = playerView.get<jl::Camera>(entity);

                    // if(entity == theScene.myPlayerIndex)
                    // {
                    //     std::cout << "camera: " << camera.transform.position.x << " " << camera.transform.position.y << " " << camera.transform.position.z << std::endl;
                    // }

                    auto & physicsComponent = playerView.get<PhysicsComponent>(entity);
                    auto & collisionCage = physicsComponent.collisionCage;
                    auto & animation_state = renderComponent.animation_state;
                    auto & controls = playerView.get<Controls>(entity);
                    auto id = entity;


                    billboard.position = camera.transform.position;
                    billboard.direction = camera.transform.direction;
                    billboard.characterNum = static_cast<int>(id) % 4;

                    profiler.checkTime("Check and set some vars for a player");


                    collisionCage.updateToSpot(&world, camera.transform.position, deltaTime);

                    profiler.checkTime("Update collcage for a player");
                    camera.updateWithYawPitch(camera.transform.yaw, camera.transform.pitch);
                    profiler.checkTime("Update yp for a player");

                    profiler.checkTime("Interp toward target yp");


                    float wantedAnim = IDLE;
                    if(controls.anyMovement())
                    {
                        if(controls.jump)
                        {
                            wantedAnim = JUMP;
                        }
                        else
                            if(controls.left && !controls.right)
                            {
                                wantedAnim = LEFTSTRAFE;
                            } else
                                if(controls.right && !controls.left)
                                {
                                    wantedAnim = RIGHTSTRAFE;
                                } else
                                    if(controls.backward && !controls.forward)
                                    {
                                        wantedAnim = BACKWARDSRUN;
                                    } else
                                    {
                                        wantedAnim = RUN;
                                    }
                    }
                    profiler.checkTime("Branch and set anim");
                       
                    float glfwtime = static_cast<float>(glfwGetTime());
                    auto existinganim = &animation_state;
                    AnimationState newState{
                        wantedAnim,
                        glfwtime,
                        static_cast<AnimationName>(wantedAnim) == JUMP ? 0.5f : 1.0f
                    };

                    bool jumpinprogress = (static_cast<AnimationName>(existinganim->actionNum) == JUMP) && (glfwtime - existinganim->timestarted < 0.75f);
                    if(existinganim->actionNum != wantedAnim && !jumpinprogress)
                    {
                        animation_state = newState;
                    }

                    profiler.checkTime("Set some more stuff");


                    if (id != theScene.myPlayerIndex)
                    {
                        billboards.push_back(billboard);
                        animStates.push_back(animation_state);
                    }
                    profiler.checkTime("pushback billboards");

                    PlayerUpdate(deltaTime, &world, particles, renderComponent, physicsComponent, movementComponent, controls, camera, particleComponent, inventory);
                    profiler.checkTime("PlayerUpdate for a player");
                }
                profiler.checkTime("For entity playerview 1");
            }
            // else
            // {
            //     //camera intro part
            //
            //     // auto & cam = theScene.our<jl::Camera>();
            //     // cam.updateWithYawPitch(0.0f, -89.9f);
            //     //
            //     // static bool thingset = false;
            //     // static glm::vec3 origspot = {};
            //     // if(!thingset)
            //     // {
            //     //     thingset = true;
            //     //     origspot = cam.transform.position;
            //     // }
            //
            //
            //     //cam.transform.position.y = origspot.y  + (1800.0f - 1800.0f *(theScene.worldIntroTimer / (INTROFLYTIME )));
            // }


        //     for (auto & [id, player] : theScene.players)
        // {
        //
        // }


        jl::updateBillboards(billboards);
        jl::updateAnimStates(animStates);
            profiler.checkTime("Update billboards and anim states");




            DGMessage msg = {};

            while (networkToMainBlockChangeQueue.pop(&msg))
            {



                visit([&](const auto& m) {
                    using T = std::decay_t<decltype(m)>;
                    if constexpr (std::is_same_v<T, WorldInfo>) {
                        // std::cout << "Got world info " << m.seed << " \n"
                        // << "playerIndex: " << (int)m.yourPlayerIndex << " \n"
                        // << "yourPosition: " << m.yourPosition.x << " " << m.yourPosition.y << " " << m.yourPosition.z << " \n";
                        //
                        // theScene.world->setSeed(m.seed);
                        // theScene.myPlayerIndex = theScene.addPlayerWithIndex(m.yourPlayerIndex, theScene.settings.clientUID);
                        //
                        // //Dont do this yet, receive the file first
                        // //theScene.worldReceived = true;
                    }
                    else if constexpr (std::is_same_v<T, ControlsUpdate>) {
                        // std::cout << "Got a controls update from " << m.myPlayerIndex << "\n";
                        // std::cout << m.myControls << " \n";
                        // std::cout << m.startPos.x << " " << m.startPos.y << " " << m.startPos.z << "\n";
                        if(!theScene.REG.valid(m.myPlayerIndex))
                        {
                            theScene.addPlayerWithIndex(m.myPlayerIndex, m.id);
                        }
                        theScene.REG.patch<Controls>(m.myPlayerIndex, [&](auto &cont){ cont = m.myControls; });
                        //theScene.players.at(m.myPlayerIndex)->controls = m.myControls;

                        //theScene.players.at(m.myPlayerIndex)->camera.transform.position = m.startPos;
                        auto & pos = theScene.REG.get<jl::Camera>(m.myPlayerIndex).transform.position;
                        pos = m.startPos;

                        theScene.REG.get<PhysicsComponent>(m.myPlayerIndex).controller->setPosition(PxExtendedVec3(
                    m.startPos.x,
                    m.startPos.y,
                    m.startPos.z)
                        );

                        theScene.REG.get<jl::Camera>(m.myPlayerIndex).transform.yaw = m.startYawPitch.x;
                        theScene.REG.get<jl::Camera>(m.myPlayerIndex).transform.pitch = m.startYawPitch.y;
                        theScene.REG.get<jl::Camera>(m.myPlayerIndex).targetYaw = m.startYawPitch.x;
                        theScene.REG.get<jl::Camera>(m.myPlayerIndex).targetPitch = m.startYawPitch.y;
                    }
                    else if constexpr (std::is_same_v<T, YawPitchUpdate>)
                    {
                        if(!theScene.REG.valid(m.myPlayerIndex))
                        {
                            theScene.addPlayerWithIndex(m.myPlayerIndex, m.id);
                        }
                        auto & playcam = theScene.REG.get<jl::Camera>(m.myPlayerIndex);
                        playcam.transform.yaw = m.newYaw;
                        playcam.transform.pitch = m.newPitch;

                    }
                    else if constexpr (std::is_same_v<T, PlayerSelectBlockChange>)
                    {
                        if(theScene.REG.valid(m.myPlayerIndex))
                        {
                            theScene.REG.patch<InventoryComponent>(m.myPlayerIndex, [&](InventoryComponent & inv)
                            {
                                inv.currentHeldBlock = m.newMaterial;
                            });
                        }
                    }
                    else if constexpr (std::is_same_v<T, PlayerLeave>)
                    {
                        if(theScene.REG.valid(m.myPlayerIndex))
                        {
                            //theScene.REG.get<PhysicsComponent>(m.myPlayerIndex).release();
                            theScene.REG.destroy(m.myPlayerIndex);

                        }
                    }
                    else if constexpr (std::is_same_v<T, RequestInventorySwap>)
                    {
                        InventorySlot* source = nullptr;
                        InventorySlot* destination = nullptr;

                        if (m.mouseSlotS)
                        {
                            source = &(theScene.REG.get<InventoryComponent>(m.myPlayerIndex).inventory.mouseHeldItem);
                        } else
                        {
                            source = &(theScene.REG.get<InventoryComponent>(m.myPlayerIndex).inventory.inventory.at(m.sourceIndex));
                        }

                        if (m.mouseSlotD)
                        {
                            destination = &(theScene.REG.get<InventoryComponent>(m.myPlayerIndex).inventory.mouseHeldItem);
                        } else
                        {
                            destination = &(theScene.REG.get<InventoryComponent>(m.myPlayerIndex).inventory.inventory.at(m.destinationIndex));
                        }

                        if (source && destination)
                        {
                            InventorySlot srcCopy = *source;
                            *source = *destination;
                            *destination = srcCopy;
                        }
                    }
                    else if constexpr (std::is_same_v<T, PlayerPresent>) {

                        bool isInReg = false;
                        auto view = theScene.REG.view<UUIDComponent>();
                        for(auto entity : view)
                        {
                            auto uid = view.get<UUIDComponent>(entity).uuid;
                            if(uid == m.id)
                            {
                                isInReg = true;
                            }
                        }
                        if(!isInReg)
                        {
                            theScene.addPlayerWithIndex(m.index, m.id);
                        }

                        theScene.REG.get<PhysicsComponent>(m.index).controller->setPosition(PxExtendedVec3(
                        m.position.x,
                        m.position.y - CAMERA_OFFSET,
                        m.position.z)
                            );
                        // if (theScene.existingInvs.contains(m.id))
                        // {
                        //     if (auto inv = loadInvFromFile("mpworld.txt", m.id))
                        //     {
                        //         std::cout << "Loaded inv to player " << (int)m.index << " with id " << m.id << "\n";
                        //         theScene.REG.get<InventoryComponent>(m.index).inventory = inv.value();
                        //     }
                        // } else
                        // {
                        //     theScene.REG.get<InventoryComponent>(m.index).inventory.inventory = DEFAULT_INVENTORY;
                        // }
                        theScene.REG.get<jl::Camera>(m.index).transform.position = m.position;
                        theScene.REG.get<jl::Camera>(m.index).transform.direction = m.direction;
                    }
                    else if constexpr (std::is_same_v<T, BlockSet>) {
                        //std::cout << "Processing network block change \n";

                        Scene* scene = &theScene;
                        //std::cout << " Server Setting" << std::endl;
                            auto & spot = m.spot;
                            //std::cout << "At Spot: " << spot.x << ", " << spot.y << ", " << spot.z << std::endl;
                            BlockType blockThere = scene->world->get(spot); //TODO why is it locking on it maybe we can do something better here
                            glm::vec3 burstspot = glm::vec3(
                                spot.x+ 0.5,
                                spot.y + 0.5,
                                spot.z + 0.5);
                            if (scene->particles)
                            {
                                scene->particles->particleBurst(burstspot,
                                                             18, (MaterialName)blockThere, 0.8, 2.0f);
                            }



                        //If removing block entity

                                if(auto f = findEntityRemoveFunc((MaterialName)blockThere); f != std::nullopt)
                                {

                                    f.value()(theScene.REG, m.spot);
                                }

                        //If adding block entity
                            if(auto func = findEntityCreateFunc((MaterialName)(m.block & BLOCK_ID_BITS)); func != std::nullopt)
                            {

                                func.value()(theScene.REG, m.spot);
                            }



                            if (m.block == AIR || findSpecialBlockMeshFunc((MaterialName)(m.block & BLOCK_ID_BITS)) != std::nullopt)
                            {
                                auto cs = scene->worldRenderer->chunkSize;
                                auto xmod = properMod(spot.x, cs);
                                auto zmod = properMod(spot.z, cs);

                                scene->worldRenderer->requestChunkRebuildFromMainThread(
                                    spot, m.block, false, m.pp
                                    );


                                if(xmod == scene->worldRenderer->chunkSize - 1)
                                {
                                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                                        IntTup(spot.x+1, spot.y, spot.z), std::nullopt, true, glm::vec3(0.f), true, true);

                                } else if(xmod == 0)
                                {
                                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                                        IntTup(spot.x-1, spot.y, spot.z), std::nullopt, true, glm::vec3(0.f), true, true);
                                }

                                if(zmod == scene->worldRenderer->chunkSize - 1)
                                {
                                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                                        IntTup(spot.x, spot.y, spot.z+1), std::nullopt, true, glm::vec3(0.f), true, true);

                                } else if(zmod == 0)
                                {
                                    scene->worldRenderer->requestChunkRebuildFromMainThread(
                                        IntTup(spot.x, spot.y, spot.z-1), std::nullopt, true, glm::vec3(0.f), true, true);
                                }
                                scene->worldRenderer->requestChunkRebuildFromMainThread(
                                    spot, std::nullopt, true, glm::vec3(0.f), true, false
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
            profiler.checkTime("Popped a network message");





            if (simscene)
            {
                if (deltaTime > 0.0f)
                {
                    gScene->simulate(glm::min(0.5f, deltaTime), nullptr, scratchMemory, scratchMemorySize);
                }

                //std::cout << "Simming scene with seed " <<
            }
            profiler.checkTime("Start simulate");




            auto & camera = theScene.our<jl::Camera>();




            setListenerToCameraPos(&camera);

            profiler.checkTime("Set listener pos");

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
                        TwoIntTup cposhere = theScene.worldRenderer->stupidWorldRendererWorldToChunkPos(TwoIntTup(x, z));
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

            profiler.checkTime("Rebuild area notifs");

            TwoIntTup popped;
            if (lightOverlapNotificationQueue.try_pull(popped) == boost::queue_op_status::success)
            {
                theScene.worldRenderer->requestChunkSpotRebuildFromMainThread(popped, false);
            }
            profiler.checkTime("Light overlap notifs");

            Atmosphere currAtmos = skyAndFogColor(theScene.currentPlanetType);

            if (theScene.bulkPlaceGizmo->active)
            {
                auto thespot = camera.transform.position + (camera.transform.direction * 3.0f);
                auto theintspot = IntTup(thespot.x, thespot.y, thespot.z);
                theScene.bulkPlaceGizmo->corner2 = theintspot;
            }

            dgDrawSky(theScene.our<jl::Camera>().transform.position, lutTexture, world, theScene.timeOfDay, &theScene.our<jl::Camera>());
            


            if (theScene.our<jl::Camera>().transform.position.y > 300.0f)
            {
                dgDrawSky(theScene.our<jl::Camera>().transform.position, lutTexture, world, 900.0f, &theScene.our<jl::Camera>());
                //
                // glUseProgram(gltfShader.shaderID);
                //
                // glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
                // glActiveTexture(GL_TEXTURE0);
                //
                // glBindTexture(GL_TEXTURE_2D, planet.texids.at(0));
                //
                //
                // glUniform1i(glGetUniformLocation(gltfShader.shaderID, "texture1"), 0);
                //
                // auto down = glm::vec3(0.f, -1.f, 0.f);
                //
                // glm::vec3 posToRenderAt = camera.transform.position + (down * 1700.0f) + (down * camera.transform.position.y );
                //
                // glUniform3f(glGetUniformLocation(gltfShader.shaderID, "pos"), posToRenderAt.x, posToRenderAt.y, posToRenderAt.z);
                // glUniform1f(glGetUniformLocation(gltfShader.shaderID, "rot"), theScene.worldIntroTimer * 5.0f);
                // glUniform1f(glGetUniformLocation(gltfShader.shaderID, "hideClose"), 1.0f);
                // glUniform3f(glGetUniformLocation(gltfShader.shaderID, "camPos"), camera.transform.position.x, camera.transform.position.y, camera.transform.position.z);
                //
                // for(jl::ModelGLObjects &mglo : planet.modelGLObjects)
                // {
                //     glBindVertexArray(mglo.vao);
                //     //Indent operations on this vertex array object
                //     glDrawElements(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr);
                //
                //     glBindVertexArray(0);
                // }
            }

            // if (theScene.worldIntroTimer < INTROFLYTIME)
            // {
            //
            //     dgDrawSky(theScene.our<jl::Camera>().transform.position, lutTexture, world, 900.0f);
            //
            //     //drawFullscreenKaleidoscope();
            //     //glClear(GL_DEPTH_BUFFER_BIT);
            //     {
            //         glUseProgram(gltfShader.shaderID);
            //
            //         glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
            //         glActiveTexture(GL_TEXTURE0);
            //
            //         glBindTexture(GL_TEXTURE_2D, planet.texids.at(0));
            //
            //
            //         glUniform1i(glGetUniformLocation(gltfShader.shaderID, "texture1"), 0);
            //
            //         auto camdir = camera.transform.direction;
            //
            //         glm::vec3 posToRenderAt = camera.transform.position + (camdir * 1950.0f) + (camdir * (1800.0f - 1800.0f *(theScene.worldIntroTimer / (INTROFLYTIME ))));
            //
            //         glUniform3f(glGetUniformLocation(gltfShader.shaderID, "pos"), posToRenderAt.x, posToRenderAt.y, posToRenderAt.z);
            //         glUniform1f(glGetUniformLocation(gltfShader.shaderID, "rot"), theScene.worldIntroTimer * 5.0f);
            //         glUniform1f(glGetUniformLocation(gltfShader.shaderID, "hideClose"), 1.0f);
            //         glUniform3f(glGetUniformLocation(gltfShader.shaderID, "camPos"), camera.transform.position.x, camera.transform.position.y, camera.transform.position.z);
            //
            //         for(jl::ModelGLObjects &mglo : planet.modelGLObjects)
            //         {
            //             glBindVertexArray(mglo.vao);
            //             //Indent operations on this vertex array object
            //             glDrawElements(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr);
            //
            //             glBindVertexArray(0);
            //         }
            //     }
            // }
            profiler.checkTime("Some crap");

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
            static GLuint odffLoc = glGetUniformLocation(mainShader.shaderID, "overridingDewyFogFactor");
            static GLuint wcaLoc = glGetUniformLocation(mainShader.shaderID, "worldCurveAmount");
            static GLuint uwloc = glGetUniformLocation(mainShader.shaderID, "underwater");
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(camera.mvp));
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(texture1Loc, 0);
            glUniform1f(abLoc, ambBrightFromTimeOfDay(theScene.timeOfDay, theScene.dayLength));
            glUniform3f(posLoc, 0.0, 0.0, 0.0);
            const glm::vec3 campos = theScene.our<jl::Camera>().transform.position;
            glUniform3f(camPosLoc, campos.x, campos.y, campos.z);
            glUniform1f(rotLoc, 0.0f);
            glUniform3f(offsetLoc, 0.0f, 0.0f, 0.0f);
            glUniform1f(scaleLoc, 1.0f);
            glUniform1f( wcaLoc, campos.y > 230.f ? glm::max(0.f, std::min(50.f, campos.y - 230.f)) / 50.f : 0.f );
            glUniform1f(uwloc, theScene.blockHeadIn == WATER ? 1.0f : 0.0f);
            auto ourCam = theScene.our<jl::Camera>().transform.position;
            IntTup itspot(
                ourCam.x,
                ourCam.y,
                ourCam.z
            );

            float temperature_noise = world.worldGenMethod->getTemperatureNoise(itspot);
            float humidity_noise = world.worldGenMethod->getHumidityNoise(itspot);
            float dewyFogFactorAtCam = theScene.worldRenderer->getDewyFogFactor(temperature_noise, humidity_noise);


            glUniform1f(odffLoc, dewyFogFactorAtCam);

            glUniform3f(fogColLoc, currAtmos.fogColor.x, currAtmos.fogColor.y, currAtmos.fogColor.z);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_3D, lutTexture);
            glUniform1i(lutLoc, 1);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, worldTex.id);
            profiler.checkTime("Send some uniforms");

            //std::cout << "World intro time" << theScene.worldIntroTimer << std::endl;
            renderer->mainThreadDraw(&theScene.our<jl::Camera>(), mainShader.shaderID, world.worldGenMethod, deltaTime, true);
            profiler.checkTime("Draw from main thread");
            vms->draw(&world, theScene.myPlayerIndex, theScene.REG);
            profiler.checkTime("Draw voxel model stamper");
            glUniform1f(timeRenderedLoc, 10.0f);
            glUniform1f(scaleLoc, 0.4f);

            glUseProgram(mainShader.shaderID);

            glUniform1f(odffLoc, dewyFogFactorAtCam);

            glUniform3f(fogColLoc, currAtmos.fogColor.x, currAtmos.fogColor.y, currAtmos.fogColor.z);

            glUniform3f(offsetLoc, -0.5f, -0.5f, -0.5f);

            for (auto entity : playerView)
            {
                auto id = entity;

                auto & inventory = playerView.get<InventoryComponent>(entity);
                auto & camera = playerView.get<jl::Camera>(entity);
                auto & controls = playerView.get<Controls>(entity);
                auto & invComp = playerView.get<InventoryComponent>(entity);
                auto & renderComp = playerView.get<RenderComponent>(entity);

                auto equippeditems = inventory.inventory.getEquippedItems();
                bool jpEquipped = false;
                for (auto& equippeditem : equippeditems)
                {
                    if (equippeditem.block == ItemName::JETPACK)
                    {
                        jpEquipped = true;
                    }
                }
                auto pos = camera.transform.position + (camera.transform.direction*0.5f) + (camera.transform.right * 0.5f);
                pos.y -= 0.5f;

                
                bool isme = (id == theScene.myPlayerIndex);
                // if (isme)
                // {
                //     //std::cout << "isme " << std::endl;
                //     glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(theScene.guiCamera->mvp));
                //     glUniform3f(posLoc, 3.0, -1.2, std::min((float)wwi / 600.0f, 2.0f));
                //     glDisable(GL_DEPTH_TEST);
                //     glUniform3f(camPosLoc, 0, 0, 0);
                // } else
                {

                    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(theScene.our<jl::Camera>().mvp));
                    glUniform3f(posLoc, pos.x, pos.y, pos.z);
                }

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, worldTex.id);

                // if(isme)
                // {
                //     drawHandledBlock(theScene.guiCamera->transform.position, invComp.currentHeldBlock, mainShader.shaderID, invComp.lastHeldBlock, renderComp.handledBlockMeshInfo);
                //
                // } else
                // {
                //drawHandledBlock(camera.transform.position, invComp.currentHeldBlock, mainShader.shaderID, invComp.lastHeldBlock, renderComp.handledBlockMeshInfo);

                //}

               // glEnable(GL_DEPTH_TEST);

                if (jpEquipped && !isme)
                {
                    {
                        glUseProgram(gltfShader.shaderID);

                        //glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
                        //glActiveTexture(GL_TEXTURE0);

                        if (controls.secondary1)
                        {
                            glBindTexture(GL_TEXTURE_2D, jplit.texids.at(0));
                        }
                        else
                        {
                            glBindTexture(GL_TEXTURE_2D, jp.texids.at(0));
                        }

                        glUniform1i(glGetUniformLocation(gltfShader.shaderID, "texture1"), 0);

                        auto camdir = camera.transform.direction;
                        camdir.y = 0;
                        camdir = glm::normalize(camdir);

                        glm::vec3 posToRenderAt = camera.transform.position - (camdir * 0.3f);
                        posToRenderAt.y = camera.transform.position.y - 0.5f;
                        glUniform3f(glGetUniformLocation(gltfShader.shaderID, "pos"), posToRenderAt.x, posToRenderAt.y, posToRenderAt.z);
                        glUniform1f(glGetUniformLocation(gltfShader.shaderID, "rot"), camera.transform.yaw);
                        glUniform1f(glGetUniformLocation(gltfShader.shaderID, "hideClose"), 0.0f);
                        glUniform3f(glGetUniformLocation(gltfShader.shaderID, "camPos"),0.f, 0.f, 0.f);

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
            profiler.checkTime("For entity playerview 2");
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

            if (simscene && deltaTime > 0.0f)
            {
                gScene->fetchResults(true);
            }
            profiler.checkTime("Fetch results");



            //std::cout << "Passing " << camera.transform.position.x << " " << camera.transform.position.y << " " << camera.transform.position.z << " \n";



            glBindTexture(GL_TEXTURE_2D, worldTex.id);



            for(auto & gizmo : theScene.gizmos)
            {
                gizmo->draw(&world, theScene.myPlayerIndex, theScene.REG);
            }
            profiler.checkTime("Draw gizmos");









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
            profiler.checkTime("Draw billboards");



            drawSunAndMoon(&camera, theScene.timeOfDay, theScene.dayLength, camera.transform.position);

            drawComputerScreensInReg(&world, theScene.REG, camera);

            profiler.checkTime("Draw sun and moon and computers");

            renderImGui();

            profiler.checkTime("Render imgui");





            particles->cleanUpOldParticles(deltaTime);

            profiler.checkTime("Cleanup old particles");

            theScene.hud->draw();
            profiler.checkTime("Draw hud");

        } else
        {
            renderImGui();
        }



        glfwPollEvents();
        glfwSwapBuffers(window);

        profiler.checkTime("Swapbuffers");
    }


    glfwDestroyWindow(window);

}
