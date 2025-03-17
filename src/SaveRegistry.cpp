//
// Created by jack on 3/17/2025.
//
#include "SaveRegistry.h"
#include "FileArchives.h"
#include "components/PlayerEmplacer.h"

void saveRegistry(entt::registry & reg, const char* filename)
{
    std::ofstream output(filename, std::ios::binary);
    SnapshotOutputArchive outputArchive(output);


    entt::snapshot{reg}
    .get<entt::entity>(outputArchive)
    .get<InventoryComponent>(outputArchive)
    .get<jl::Camera>(outputArchive)
    .get<RenderComponent>(outputArchive)
    .get<PhysicsComponent>(outputArchive)
    .get<Controls>(outputArchive)
    .get<NetworkComponent>(outputArchive)
    .get<MovementComponent>(outputArchive)
    .get<ParticleEffectComponent>(outputArchive);
}


void loadRegistry(entt::registry & reg, const char* filename)
{
    std::ifstream input(filename, std::ios::binary);
    SnapshotInputArchive inputArchive(input);

    entt::snapshot_loader{reg}
    .get<entt::entity>(inputArchive)
    .get<InventoryComponent>(inputArchive)
    .get<jl::Camera>(inputArchive)
    .get<RenderComponent>(inputArchive)
    .get<PhysicsComponent>(inputArchive)
    .get<Controls>(inputArchive)
    .get<NetworkComponent>(inputArchive)
    .get<MovementComponent>(inputArchive)
    .get<ParticleEffectComponent>(inputArchive);

}