//
// Created by jack on 1/6/2025.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "Camera.h"
#include "Controls.h"

struct Player {
    jl::Camera camera;
    Controls controls;
    physx::PxController* controller;
    void update(float deltaTime);
    Player();
    ~Player();
};



#endif //PLAYER_H
