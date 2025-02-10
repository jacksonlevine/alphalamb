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
        std::cout << "about to connect \n";
        boost::asio::connect(tsocket, resolver.resolve(addr, port));
        std::cout << "connected \n";
        connected = true;
    } catch(std::exception e)
    {
        connected = false;
        std::cout << e.what() << "\n";
    }

    if (connected)
    {
        launchReceiverThread(&tsocket, &theScene.clientShouldRun);
    }

    return connected;
}

inline void exitWorld(Scene* scene)
{
    std::cout << "Stopping \n";
    scene->worldRenderer->stopThreads();
    std::cout << "Stopped \n";


    scene->worldRenderer->activeChunks.clear();
    //scene->worldRenderer->mbtActiveChunks.clear();
    scene->players.at(scene->myPlayerIndex)->controller->setPosition(DEFAULT_PLAYERPOS);
    scene->players.at(scene->myPlayerIndex)->camera.transform.position = glm::vec3(DEFAULT_PLAYERPOS.x, DEFAULT_PLAYERPOS.y, DEFAULT_PLAYERPOS.z);
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

    for (auto & chunk : scene->worldRenderer->chunkPool)
    {
        glDeleteBuffers(8, &chunk.vvbo);
        glDeleteVertexArrays(1, &chunk.drawInstructions.vao);
        glDeleteVertexArrays(1, &chunk.drawInstructions.tvao);
    }
    scene->worldRenderer->chunkPool.clear();

    scene->world->clearWorld();

    int myPlayerIndex = scene->myPlayerIndex;
    scene->players.erase(myPlayerIndex);
    scene->myPlayerIndex = -1;
}

inline void uncaptureMouse(Scene* s)
{
    glfwSetInputMode(s->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    s->mouseCaptured = false;
    s->firstMouse = true;
}



#endif //SHAREDVARSBETWEENMAINANDGUI_H
