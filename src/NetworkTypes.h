//
// Created by jack on 2/8/2025.
//

#ifndef NETWORKTYPES_H
#define NETWORKTYPES_H

#include <boost/variant2/variant.hpp>

#include "Camera.h"
#include "ClientUID.h"
#include "Controls.h"
#include "IntTup.h"
#include "world/MaterialName.h"
#include "world/VoxelModels.h"
#include "Inventory.h"

using namespace boost::variant2;

struct PlayerPresent
{
    int index;
    glm::vec3 position;
    glm::vec3 direction;
    ClientUID id;
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
    BlockType block;
};


struct YawPitchUpdate
{
    int myPlayerIndex;
    float newYaw;
    float newPitch;
};

struct PlayerLeave
{
    int myPlayerIndex;
};

struct PlayerSelectBlockChange
{
    int myPlayerIndex;
    MaterialName newMaterial;
};

struct BulkBlockSet
{
    IntTup corner1;
    IntTup corner2;
    BlockType block;
    bool hollow = false;
};

struct VoxModelStamp
{
    VoxelModelName name;
    IntTup spot;
};

struct ClientToServerGreeting
{
    ClientUID id;
};

// struct EquippedItemsUpdate
// {
//     int myPlayerIndex;
//     std::array<InventorySlot, INVHEIGHT> equipped;
// };

using DGMessage = variant<WorldInfo, ControlsUpdate, FileTransferInit, BlockSet, PlayerPresent, YawPitchUpdate, PlayerLeave, PlayerSelectBlockChange, BulkBlockSet, VoxModelStamp, ClientToServerGreeting>;




#endif //NETWORKTYPES_H
