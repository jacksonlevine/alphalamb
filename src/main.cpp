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

struct Scene
{
    std::vector<Player*> players = {};
    int myPlayerIndex = -1;
    size_t addPlayer()
    {
        auto index = players.size();
        players.push_back(new Player());
        return index;
    }
    bool mouseCaptured = false;
    bool firstMouse = true;
    std::vector<WorldGizmo*> gizmos;
    ParticlesGizmo* particles = nullptr;
};

Scene theScene = {};

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
    if(action)
    {
        if(button == GLFW_MOUSE_BUTTON_LEFT)
        {

            if(!scene->mouseCaptured)
            {
                // if(imguiio->WantCaptureMouse)
                // {
                //     return;
                // }
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                scene->mouseCaptured = true;
            } else
            {
                //addShootLine();
                //sendShootLineMessage();
                // auto & cam = scene->players[scene->myPlayerIndex]->camera;
                //
                // scene->particles->particleBurst(cam.transform.position + cam.transform.direction * 3.0f,
                //                                 20, WOOD_PLANKS, 2.0, 1.0f);
            }

        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            scene->mouseCaptured = false;
            scene->firstMouse = true;
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
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
        if(key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(window, 1);
        }
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
        const double xOffset = (xpos - lastx) * (30.0f/100.0f);
        const double yOffset = (lasty - ypos) * (30.0f/100.0f);

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
}

int main()
{

    glfwInit();
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "project7", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
    };

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

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

    // int width, height;
    // glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, mode->width, mode->height);


    float deltaTime = 0.0f;

    glfwSetWindowUserPointer(window, &theScene);

    initializeImGui(window);
    ImFont* font_title = imguiio->Fonts->AddFontFromFileTTF("font.ttf", 20.0f, NULL, imguiio->Fonts->GetGlyphRangesDefault());

    ImFont* font_body = imguiio->Fonts->AddFontFromFileTTF("font.ttf", 20.0f, NULL, imguiio->Fonts->GetGlyphRangesDefault());



    //Add ourselves to the scene
    theScene.myPlayerIndex = theScene.addPlayer();

    {
        auto & camera = theScene.players.at(theScene.myPlayerIndex)->camera;
        camera.updateProjection(mode->width, mode->height, 90.0f);
    }

    ParticlesGizmo* particles = new ParticlesGizmo();
    theScene.particles = particles;

    theScene.gizmos.push_back(particles);
    theScene.gizmos.push_back(new BlockSelectGizmo());


    for(auto & gizmo : theScene.gizmos)
    {
        gizmo->init();
    }

    World world(
        new HashMapDataMap(),
        new OverworldWorldGenMethod(),
        new HashMapDataMap());
    VoxModel swcModel = loadSwc("resources/swctest.txt");
    stampVoxelModelInWorld(&world,swcModel);


    WorldRenderer renderer;

    std::thread chunkWorker(&WorldRenderer::meshBuildCoroutine, &renderer,
        &(theScene.players[theScene.myPlayerIndex]->camera), &world);
    chunkWorker.detach();


    while (!glfwWindowShouldClose(window))
    {
        static float lastTime = glfwGetTime();
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto & player : theScene.players)
        {
            player->update(deltaTime, &world, particles);
        }

        static jl::Shader gltfShader = getBasicShader();
        static jl::Texture worldTex("resources/world.png");



        if (theScene.myPlayerIndex != -1)
        {

            gScene->simulate(deltaTime);



            auto & camera = theScene.players.at(theScene.myPlayerIndex)->camera;
            camera.updateWithYawPitch(camera.transform.yaw, camera.transform.pitch);

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

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(camera.mvp));
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(texture1Loc, 0);
            glUniform3f(posLoc, 0.0, 0.0, 0.0);
            const glm::vec3 campos = theScene.players[theScene.myPlayerIndex]->camera.transform.position;
            glUniform3f(camPosLoc, campos.x, campos.y, campos.z);
            glUniform1f(rotLoc, 0.0f);
            glUniform1f(grcLoc, 0.0f);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_3D, lutTexture);
            glUniform1i(lutLoc, 1);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, worldTex.id);

            if(FLY_MODE)
            {
                std::cout << "" << camera.transform.position.x << " " << camera.transform.position.y << " " << camera.transform.position.z << " \n";
            }

            renderer.mainThreadDraw(&theScene.players[theScene.myPlayerIndex]->camera, gltfShader.shaderID, world.worldGenMethod);


            gScene->fetchResults(true);


            //std::cout << "Passing " << camera.transform.position.x << " " << camera.transform.position.y << " " << camera.transform.position.z << " \n";
            theScene.players[theScene.myPlayerIndex]->collisionCage.updateToSpot(&world, camera.transform.position);


            glBindTexture(GL_TEXTURE_2D, worldTex.id);

            for(auto & gizmo : theScene.gizmos)
            {
                gizmo->draw(&world, theScene.players[theScene.myPlayerIndex]);
            }

            particles->cleanUpOldParticles(deltaTime);

            renderImGui();
        }


        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);

}