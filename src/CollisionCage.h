//
// Created by jack on 1/27/2025.
//

#ifndef COLLISIONCAGE_H
#define COLLISIONCAGE_H

#include "PrecompHeader.h"
#include "world/World.h"
#include "world/WorldRenderer.h"

using namespace physx;

//#define DEBUGDRAW 1

class CollisionCage {
public:
    void updateToSpot(World* world, glm::vec3 spot);
private:
    IntTup lastBlockSpot = IntTup(-9999,9999,-9999);
    PxRigidStatic* collider = nullptr;
#ifdef DEBUGDRAW
    ChunkGLInfo cgl = {};
#endif
};



#endif //COLLISIONCAGE_H
