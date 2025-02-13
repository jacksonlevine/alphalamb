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
    ChunkGLInfo handledBlockMeshInfo = {};
    Billboard billboard = {};
    CollisionCage collisionCage = {};
    AnimationState animation_state = {};
    physx::PxController* controller = nullptr;
    Controls controls = {};
    void update(float deltaTime, World* world, ParticlesGizmo* particles);
    MaterialName lastHeldBlock = AIR;
    MaterialName currentHeldBlock = WOOD_PLANKS;
    float footDustTimer = 0.0f;
    MaterialName lastBlockStandingOn = AIR;
    bool isGrounded = false;
    bool jetpackMode = false;
    bool hoverMode = false;
    MyControllerHitReport* getMyHitReport()
    {
        return ((MyControllerHitReport*)controller->getUserData());
    }
    Player();
    ~Player();
};



#endif //PLAYER_H
