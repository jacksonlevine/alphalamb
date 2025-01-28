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
};

Scene theScene = {};


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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    initializePhysX();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);


    float deltaTime = 0.0f;

    glfwSetWindowUserPointer(window, &theScene);

    //Add ourselves to the scene
    theScene.myPlayerIndex = theScene.addPlayer();

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
            glUseProgram(gltfShader.shaderID);

            static GLuint mvpLoc = glGetUniformLocation(gltfShader.shaderID, "mvp");
            static GLuint texture1Loc = glGetUniformLocation(gltfShader.shaderID, "texture1");
            static GLuint posLoc = glGetUniformLocation(gltfShader.shaderID, "pos");
            static GLuint rotLoc = glGetUniformLocation(gltfShader.shaderID, "rot");

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(camera.mvp));
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(texture1Loc, 0);
            glUniform3f(posLoc, 0.0, 0.0, 0.0);
            glUniform1f(rotLoc, 0.0f);


            glBindTexture(GL_TEXTURE_2D, worldTex.id);

        }

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);

}