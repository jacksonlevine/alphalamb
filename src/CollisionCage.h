//
// Created by jack on 1/27/2025.
//

#ifndef COLLISIONCAGE_H
#define COLLISIONCAGE_H

#include "BasicShader.h"
#include "BasicShader.h"
#include "PrecompHeader.h"
#include "world/World.h"
#include "world/WorldRenderer.h"

using namespace physx;

//#define DEBUGDRAW 1

class CollisionCage {
public:
    void updateToSpot(World* world, glm::vec3 spot, float deltaTime);
    CollisionCage() = default;
    ~CollisionCage();
    CollisionCage(const CollisionCage&) = delete;
    CollisionCage& operator=(const CollisionCage&) = delete;
    CollisionCage(CollisionCage&& other)
    {
        this->collider = other.collider;
        this->tcollider = other.tcollider;
        this->lastBlockSpot = other.lastBlockSpot;
        other.collider = nullptr;
        other.tcollider = nullptr;
    }
    CollisionCage& operator=(CollisionCage&& other)
    {
        this->collider = other.collider;
        this->tcollider = other.tcollider;
        this->lastBlockSpot = other.lastBlockSpot;
        other.collider = nullptr;
        other.tcollider = nullptr;
        return *this;
    }
    PxRigidStatic* collider = nullptr;
    PxRigidStatic* tcollider = nullptr;
private:
    IntTup lastBlockSpot = IntTup(-9999,9999,-9999);

    float updateTimer = 0.0f;
#ifdef DEBUGDRAW
    ChunkGLInfo cgl = {};
#endif
};



#endif //COLLISIONCAGE_H
