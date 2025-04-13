//
// Created by jack on 2/9/2025.
//

#ifndef SHAREDVARSBETWEENMAINANDGUI_H
#define SHAREDVARSBETWEENMAINANDGUI_H


#include "PrecompHeader.h"
#include "Scene.h"

#include "Client.h"

extern boost::asio::io_context io_context;
extern boost::asio::ip::tcp::socket tsocket;
extern boost::asio::ip::tcp::resolver resolver;


void enterWorld(Scene* s);

inline bool connectToServer(const char* addr, const char* port)
{
    bool connected = false;

    using boost::asio::ip::tcp;

    try
    {
        // Connect to the server

        boost::asio::connect(tsocket, resolver.resolve(addr, port));

        connected = true;
    } catch(std::exception e)
    {
        connected = false;

    }

    if (connected)
    {
        launchReceiverThread(&tsocket, &theScene.clientShouldRun);
        launchSenderThread(&tsocket, &theScene.clientShouldRun);
    }

    return connected;
}

inline void exitWorld(Scene* scene)
{

    scene->clientShouldRun.store(false);
    scene->worldRenderer->stopThreads();


    if(scene->multiplayer)
    {
        scene->REG.clear();
        //Prob need to do something w this
        // for(auto & player : scene->players)
        // {
        //
        //
        //     player.second->controller->release();
        //     player.second->collisionCage.collider->release();
        // }
        // scene->players.clear();
    }

    {
        std::unique_lock<std::shared_mutex> lock(scene->world->placedVoxModels.mutex);
        scene->world->placedVoxModels.models.clear();
    }
    {
        std::unique_lock<std::shared_mutex> lock(scene->world->blockAreas.baMutex);
        scene->world->blockAreas.blockAreas.clear();
    }



    scene->worldRenderer->activeChunks.clear();
    scene->worldRenderer->mbtActiveChunks.clear();
    scene->worldRenderer->generatedChunks.clear();

    for (auto & c : scene->worldRenderer->changeBuffers)
    {
        c.in_use = false;
        c.ready = false;
    }
    for (auto & c : scene->worldRenderer->userChangeMeshBuffers)
    {
        c.in_use = false;
        c.ready = false;
    }

    size_t index;
    while (scene->worldRenderer->freedChangeBuffers.pop(index)){}
    while (scene->worldRenderer->freedUserChangeMeshBuffers.pop(index)){}
    TwoIntTup t;
    while (scene->worldRenderer->confirmedActiveChunksQueue.pop(t)){}

    //Not needed since we will be keeping these buffers 'statically'
    // for (auto & chunk : scene->worldRenderer->chunkPool)
    // {
    //     glDeleteBuffers(8, &chunk.vvbo);
    //     glDeleteVertexArrays(1, &chunk.drawInstructions.vao);
    //     glDeleteVertexArrays(1, &chunk.drawInstructions.tvao);
    // }
    // scene->worldRenderer->chunkPool.clear();

    scene->world->clearWorld();
    scene->REG.clear();
    //
    // entt::entity myPlayerIndex = scene->myPlayerIndex;
    // scene->players.erase(myPlayerIndex);
    // scene->myPlayerIndex = -1;

    //scene->REG.destroy(scene->myPlayerIndex);
    scene->myPlayerIndex = entt::null;
}

inline void uncaptureMouse(Scene* s)
{
    glfwSetInputMode(s->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    s->mouseCaptured = false;
    s->firstMouse = true;
}

inline void captureMouse(Scene* s)
{
    glfwSetInputMode(s->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    s->mouseCaptured = true;
}

inline void toggleFullscreen(GLFWwindow* window) {
    static int windowedWidth, windowedHeight;
    if (glfwGetWindowMonitor(window)) {
        glfwSetWindowMonitor(window, nullptr, 100, 100, windowedWidth, windowedHeight, GLFW_DONT_CARE);
    } else {
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
}

#endif //SHAREDVARSBETWEENMAINANDGUI_H
