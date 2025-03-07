//
// Created by jack on 2/9/2025.
//

#include "Server.h"

#include "world/datamapmethods/HashMapDataMap.h"
#include "world/worldgenmethods/OverworldWorldGenMethod.h"

std::unordered_map<int, ServerPlayer> clients;

std::shared_mutex clientsMutex;
//
// BlockAreaRegistry serverWorld.blockAreas;
// PlacedVoxModelRegistry serverWorld.placedVoxModels;
// DataMap* serverWorld.userDataMap = new HashMapDataMap();

World serverWorld{new HashMapDataMap(), new OverworldWorldGenMethod(), new HashMapDataMap()};
InvMapKeyedByUID invMapKeyedByUID = {};