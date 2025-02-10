//
// Created by jack on 2/9/2025.
//

#include "Server.h"

std::unordered_map<int, ServerPlayer> clients;

std::shared_mutex clientsMutex;