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
#include "world/World.h"
#include "world/WorldGizmo.h"
#include "world/WorldRenderer.h"
#include "world/gizmos/BlockSelectGizmo.h"
#include "world/datamapmethods/HashMapDataMap.h"
#include "world/worldgenmethods/OverworldWorldGenMethod.h"

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
        if (key == GLFW_KEY_W)
        {
            scene->players.at(scene->myPlayerIndex)->controls.forward = action;
        }
        if (key == GLFW_KEY_A)
        {
            scene->players.at(scene->myPlayerIndex)->controls.left = action;
        }
        if (key == GLFW_KEY_S)
        {
            scene->players.at(scene->myPlayerIndex)->controls.backward = action;
        }
        if (key == GLFW_KEY_D)
        {
            scene->players.at(scene->myPlayerIndex)->controls.right = action;
        }
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
    GLFWwindow* window = glfwCreateWindow(1280, 1024, "Window", nullptr, nullptr);
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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.5f, 0.5f, 1.0f, 1.0f);

    initializePhysX();

    loadGameVoxelModels();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);


    float deltaTime = 0.0f;

    glfwSetWindowUserPointer(window, &theScene);

    //Add ourselves to the scene
    theScene.myPlayerIndex = theScene.addPlayer();

    theScene.gizmos.push_back(new BlockSelectGizmo());

    for(auto & gizmo : theScene.gizmos)
    {
        gizmo->init();
    }

    World world(
        new HashMapDataMap(),
        new OverworldWorldGenMethod(),
        new HashMapDataMap());


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
            player->update(deltaTime);
        }

        static jl::Shader gltfShader = getBasicShader();
        static jl::Texture worldTex("resources/world.png");

        if (theScene.myPlayerIndex != -1)
        {
            auto & camera = theScene.players.at(theScene.myPlayerIndex)->camera;
            camera.updateWithYawPitch(camera.transform.yaw, camera.transform.pitch);

            drawSky(glm::vec4(0.3, 0.65, 1.0, 1.0),
                glm::vec4(1.0, 1.0, 1.0, 1.0),
                1.0f, &theScene.players[theScene.myPlayerIndex]->camera);


            glUseProgram(gltfShader.shaderID);

            static GLuint mvpLoc = glGetUniformLocation(gltfShader.shaderID, "mvp");
            static GLuint texture1Loc = glGetUniformLocation(gltfShader.shaderID, "texture1");
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

            glBindTexture(GL_TEXTURE_2D, worldTex.id);

            //std::cout << "Passing " << camera.transform.position.x << " " << camera.transform.position.y << " " << camera.transform.position.z << " \n";
            theScene.players[theScene.myPlayerIndex]->collisionCage.updateToSpot(&world, camera.transform.position);



            renderer.mainThreadDraw(&theScene.players[theScene.myPlayerIndex]->camera, gltfShader.shaderID, world.worldGenMethod);

            for(auto & gizmo : theScene.gizmos)
            {
                gizmo->draw(&world, theScene.players[theScene.myPlayerIndex]);
            }


        }

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);

}