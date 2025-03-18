//
// Created by jack on 3/17/2025.
//

#ifndef UUIDCOMPONENT_H
#define UUIDCOMPONENT_H
#include "../PrecompHeader.h"

struct UUIDComponent {
    ClientUID uuid;

    explicit UUIDComponent(ClientUID uid) : uuid(uid) {}

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(uuid);
    }
};

#endif //UUIDCOMPONENT_H
