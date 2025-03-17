//
// Created by jack on 3/17/2025.
//

#ifndef FILEARCHIVES_H
#define FILEARCHIVES_H

#include "PrecompHeader.h"

//Adapter from entt archive to cereal


struct SnapshotOutputArchive {
    std::ofstream& stream;
    cereal::BinaryOutputArchive archive;

    explicit SnapshotOutputArchive(std::ofstream& stream)
        : stream(stream), archive(stream) {}

    void operator()(entt::entity entity) {
        archive(entity);
    }

    void operator()(std::underlying_type_t<entt::entity> size) {
        archive(size);
    }

    template<typename Component>
    void operator()(const Component& component) {
        archive(component);
    }

    template<typename Component>
    void operator()(entt::entity entity, const Component& component) {
        archive(entity, component);
    }
};


struct SnapshotInputArchive {
    std::ifstream& stream;
    cereal::BinaryInputArchive archive;

    explicit SnapshotInputArchive(std::ifstream& stream)
        : stream(stream), archive(stream) {}

    void operator()(entt::entity& entity) {
        archive(entity);
    }

    void operator()(std::underlying_type_t<entt::entity>& size) {
        archive(size);
    }

    template<typename Component>
    void operator()(Component& component) {
        archive(component);
    }

    template<typename Component>
    void operator()(entt::entity& entity, Component& component) {
        archive(entity, component);
    }
};

#endif //FILEARCHIVES_H
