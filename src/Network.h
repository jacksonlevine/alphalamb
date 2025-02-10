//
// Created by jack on 2/8/2025.
//

#ifndef NETWORK_H
#define NETWORK_H

#include <boost/variant2/variant.hpp>

#include "Camera.h"
#include "Controls.h"
#include "IntTup.h"
using namespace boost::variant2;

struct PlayerPresent
{
    int index;
    jl::Camera camera;
};

struct WorldInfo
{
    int seed;
    glm::vec3 yourPosition;
    int yourPlayerIndex;
};

struct ControlsUpdate {
    int myPlayerIndex;
    Controls myControls;
    glm::vec3 startPos;
    glm::vec2 startYawPitch;
};

struct FileTransferInit
{
    size_t fileSize;
    bool isWorld;
};

struct BlockSet
{
    IntTup spot;
    uint32_t block;
};


struct YawPitchUpdate
{
    int myPlayerIndex;
    float newYaw;
    float newPitch;
};

using DGMessage = variant<WorldInfo, ControlsUpdate, FileTransferInit, BlockSet, PlayerPresent, YawPitchUpdate>;




#endif //NETWORK_H
