//
// Created by jack on 1/6/2025.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "Camera.h"
#include "CollisionCage.h"
#include "Controls.h"
#include "BillboardInstanceShader.h"

inline static const PxExtendedVec3 DEFAULT_PLAYERPOS = PxExtendedVec3(0, 200, 0);
inline static const float CAMERA_OFFSET = 0.8f;
class MyControllerHitReport;
class ParticlesGizmo;

struct Player {
    jl::Camera camera = {};
    Controls controls = {};
    physx::PxController* controller = nullptr;
    CollisionCage collisionCage = {};
    float footDustTimer = 0.0f;
    MaterialName lastBlockStandingOn = AIR;
    Billboard billboard = {};
    AnimationState animation_state = {};
    bool isGrounded = false;
    bool jetpackMode = false;
    void update(float deltaTime, World* world, ParticlesGizmo* particles);
    MaterialName lastHeldBlock = AIR;
    MaterialName currentHeldBlock = WOOD_PLANKS;
    ChunkGLInfo handledBlockMeshInfo = {};
    MyControllerHitReport* getMyHitReport()
    {
        return ((MyControllerHitReport*)controller->getUserData());
    }
    Player();
    ~Player();
};



#endif //PLAYER_H
