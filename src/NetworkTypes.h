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
#include "components/LootDrop.h"


struct PlayerPresent
{
    entt::entity index;
    glm::vec3 position;
    glm::vec3 direction;
    ClientUID id;
};

struct WorldInfo
{
    int seed;
    glm::vec3 yourPosition;
    entt::entity yourPlayerIndex;
};

struct ControlsUpdate {
    entt::entity myPlayerIndex;
    Controls myControls;
    glm::vec3 startPos;
    glm::vec2 startYawPitch;
    ClientUID id;
};

struct FileTransferInit
{
    size_t fileSize;
    bool isWorld;
    size_t regFileSize = 0;
};


struct AddLootDrop
{
    LootDrop lootDrop;
    glm::vec3 spot;
    entt::entity newEntityName = entt::null;
};

struct BlockSet
{
    IntTup spot;
    BlockType block;
    glm::vec3 pp;
    entt::entity newEntityNameIfApplicable = entt::null;
    std::optional<AddLootDrop> addLootDrop = std::nullopt;
};


struct YawPitchUpdate
{
    entt::entity myPlayerIndex;
    float newYaw;
    float newPitch;
    ClientUID id;
};

struct PlayerLeave
{
    entt::entity myPlayerIndex;
};

struct PlayerSelectBlockChange
{
    entt::entity myPlayerIndex;
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

struct RequestInventoryTransfer
{
    static_assert(INVWIDTH * INVHEIGHT < std::numeric_limits<uint8_t>::max(), "Inventory is too big for the index types on RequestInventoryTransfer update them");
    ClientUID sourceID;
    ClientUID destinationID;
    uint8_t sourceIndex;
    uint8_t destinationIndex;
    bool mouseSlotS;
    bool mouseSlotD;
};

struct RequestInventorySwap
{
    static_assert(INVWIDTH * INVHEIGHT < std::numeric_limits<uint8_t>::max(), "Inventory is too big for the index types on RequestInventoryTransfer update them");
    ClientUID sourceID;
    ClientUID destinationID;
    entt::entity myPlayerIndex;
    uint8_t sourceIndex;
    uint8_t destinationIndex;
    bool mouseSlotS;
    bool mouseSlotD;
};



struct RequestStackSlotsToDest
{
    ClientUID sourceID;
    ClientUID destinationID;
    entt::entity myPlayerIndex;
    uint8_t sourceIndex;
    uint8_t destinationIndex;
    bool mouseSlotS;
    bool mouseSlotD;
};

struct TextChunkHeader
{
    size_t numChunks;
};

struct TextChunk
{
    size_t sequenceNumber;
    std::array<char, 32> data;
};

struct RequestTextChunkResend
{
    size_t sequenceNumber;
};


struct PickUpLootDrop
{
    entt::entity myPlayerIndex;
    entt::entity lootDrop;
};

struct DepleteInventorySlot
{
    entt::entity playerIndex;
    int slot;
    int depletion;
};

struct BlockSetAndDepleteSlot
{
    BlockSet blockSet;
    DepleteInventorySlot deplete;
};

struct HeartbeatAndCleanup
{
    float timeOfDay;
};

// struct EquippedItemsUpdate
// {
//     entt::entity myPlayerIndex;
//     std::array<InventorySlot, INVHEIGHT> equipped;
// };

using DGMessage = std::variant<HeartbeatAndCleanup, RequestStackSlotsToDest, BlockSetAndDepleteSlot, DepleteInventorySlot, TextChunkHeader, TextChunk, RequestTextChunkResend, RequestInventorySwap, RequestInventoryTransfer, WorldInfo, ControlsUpdate, FileTransferInit, BlockSet, PlayerPresent, YawPitchUpdate, PlayerLeave, PlayerSelectBlockChange, BulkBlockSet, VoxModelStamp, ClientToServerGreeting, AddLootDrop, PickUpLootDrop>;


 const BlockSet& getBlockSet(const auto& m) {
    if constexpr (std::is_same_v<std::decay_t<decltype(m)>, BlockSetAndDepleteSlot>) {
        return m.blockSet;
    } else {
        return m;
    }
}

 const DepleteInventorySlot& getDepleteSlot(const auto& m) {
    if constexpr (std::is_same_v<std::decay_t<decltype(m)>, BlockSetAndDepleteSlot>) {
        return m.deplete;
    } else {
        return m;
    }
}


#endif //NETWORKTYPES_H
