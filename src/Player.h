//
// Created by jack on 1/6/2025.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "Camera.h"
#include "CollisionCage.h"
#include "Controls.h"


class MyControllerHitReport;
class ParticlesGizmo;

struct Player {
    jl::Camera camera;
    Controls controls;
    physx::PxController* controller;
    CollisionCage collisionCage = {};
    float footDustTimer = 0.0f;
    MaterialName lastBlockStandingOn = AIR;
    void update(float deltaTime, World* world, ParticlesGizmo* particles);
    MyControllerHitReport* getMyHitReport()
    {
        return ((MyControllerHitReport*)controller->getUserData());
    }
    Player();
    ~Player();
};



#endif //PLAYER_H
