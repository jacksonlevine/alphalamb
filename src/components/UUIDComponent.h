//
// Created by jack on 3/17/2025.
//

#ifndef UUIDCOMPONENT_H
#define UUIDCOMPONENT_H
#include "../PrecompHeader.h"

struct UUIDComponent {
    ClientUID uuid = boost::uuids::nil_uuid();

    explicit UUIDComponent(ClientUID uid) : uuid(uid) {}
    UUIDComponent() = default;

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(uuid);
    }
};

#endif //UUIDCOMPONENT_H
