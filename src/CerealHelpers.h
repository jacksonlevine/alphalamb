//
// Created by jack on 3/17/2025.
//

#ifndef CEREALHELPERS_H
#define CEREALHELPERS_H

#include "PrecompHeader.h"
#include "world/MaterialName.h"

namespace cereal {
    template<class Archive>
    void serialize(Archive& archive, glm::vec3& vec) {
        archive(vec.x, vec.y, vec.z);
    }

    template <class Archive>
    void serialize(Archive& ar, boost::uuids::uuid& uuid) {
        ar(cereal::make_nvp("uuid", uuid.data));
    }
}


#endif //CEREALHELPERS_H
