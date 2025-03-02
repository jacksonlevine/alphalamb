//
// Created by jack on 2/27/2025.
//

#ifndef PLAYERINFOMAPKEYEDBYUID_H
#define PLAYERINFOMAPKEYEDBYUID_H

#include "ClientUID.h"
#include "Inventory.h"
#include "PrecompHeader.h"


struct InvMapKeyedByUID
{
    std::map<ClientUID, Inventory> invMap = {};
    std::shared_mutex rw = {};

    std::pair<std::shared_lock<std::shared_mutex>, Inventory&> getRead(const ClientUID id) {
        std::shared_lock<std::shared_mutex> lock(rw);
        return std::make_pair(std::move(lock), std::ref(invMap[id]));
    }

    std::pair<std::unique_lock<std::shared_mutex>, Inventory&> getWrite(const ClientUID id) {
        std::unique_lock<std::shared_mutex> lock(rw);
        return std::make_pair(std::move(lock), std::ref(invMap[id]));
    }
};

#endif //PLAYERINFOMAPKEYEDBYUID_H
