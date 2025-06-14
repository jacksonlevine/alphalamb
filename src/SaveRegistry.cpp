//
// Created by jack on 3/17/2025.
//
#include "SaveRegistry.h"
#include "FileArchives.h"
#include "components/ComputerComponent.h"
#include "components/JungleCampSpawnedInChunk.h"
#include "components/Lifetime.h"
#include "components/LootDrop.h"
#include "components/Orange1.h"
#include "components/PlayerEmplacer.h"
#include "components/WorldStateComponent.h"

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
    .get<ParticleEffectComponent>(outputArchive)
    .get<UUIDComponent>(outputArchive)
    .get<ComputerComponent>(outputArchive)
    .get<NPPositionComponent>(outputArchive)
    .get<WorldState>(outputArchive)
    .get<LootDrop>(outputArchive)
    .get<Lifetime>(outputArchive)
    .get<PlayerComp>(outputArchive)
    .get<Orange1>(outputArchive)
    .get<ChunkCamps>(outputArchive)
    .get<HealthComponent>(outputArchive)
    ;
}


void loadRegistry(entt::registry & reg, const char* filename)
{

    if (std::filesystem::exists(filename))
    {

        //reg.clear();

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
        .get<ParticleEffectComponent>(inputArchive)
        .get<UUIDComponent>(inputArchive)
        .get<ComputerComponent>(inputArchive)
        .get<NPPositionComponent>(inputArchive)
        .get<WorldState>(inputArchive)
        .get<LootDrop>(inputArchive)
        .get<Lifetime>(inputArchive)
        .get<PlayerComp>(inputArchive)
        .get<Orange1>(inputArchive)
        .get<ChunkCamps>(inputArchive)
        .get<HealthComponent>(inputArchive)
        .orphans()
        ;

    }

}