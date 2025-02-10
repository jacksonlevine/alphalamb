//
// Created by jack on 2/8/2025.
//

#ifndef NETWORK_H
#define NETWORK_H

#include <boost/variant2/variant.hpp>

#include "Controls.h"
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
};

using DGMessage = variant<WorldInfo, ControlsUpdate, FileTransferInit>;




#endif //NETWORK_H
