//
// Created by jack on 2/9/2025.
//

#include "Server.h"

#include "world/datamapmethods/HashMapDataMap.h"

std::unordered_map<int, ServerPlayer> clients;

std::shared_mutex clientsMutex;

BlockAreaRegistry serverBAR;
PlacedVoxModelRegistry serverPVMR;
DataMap* serverUserDataMap = new HashMapDataMap();