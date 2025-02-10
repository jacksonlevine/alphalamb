//
// Created by jack on 2/8/2025.
//

#ifndef NETWORK_H
#define NETWORK_H

#include <boost/variant2/variant.hpp>

#include "Controls.h"
#include "IntTup.h"
using namespace boost::variant2;

struct WorldInfo
{
    int seed;
    glm::vec3 yourPosition;
    int yourPlayerIndex;
};

struct ControlsUpdate {
    int myPlayerIndex;
    Controls myControls;
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

using DGMessage = variant<WorldInfo, ControlsUpdate, FileTransferInit, BlockSet>;




#endif //NETWORK_H
