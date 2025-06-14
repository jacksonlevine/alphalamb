//
// Created by jack on 6/14/2025.
//

#ifndef JUNGLECAMPSPAWNEDINCHUNK_H
#define JUNGLECAMPSPAWNEDINCHUNK_H


#include "../IntTup.h"
#include "../PrecompHeader.h"
#include <cereal/types/vector.hpp>

struct ChunkCamps {
    std::vector<TwoIntTup> camps;

    bool has_camp(const TwoIntTup& spot) const {
        return std::find_if(camps.begin(), camps.end(),
            [spot](const auto& s) { return s == spot; }) != camps.end();
    }

    template<class Archive>
    void serialize(Archive& archive) {
        archive(camps);
    }
};

#endif //JUNGLECAMPSPAWNEDINCHUNK_H
