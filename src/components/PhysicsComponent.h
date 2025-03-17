//
// Created by jack on 3/16/2025.
//

#ifndef PHYSICSCOMPONENT_H
#define PHYSICSCOMPONENT_H

#include "../CollisionCage.h"

class PhysicsComponent {
public:
    CollisionCage collisionCage = {};
    physx::PxController* controller = nullptr;
    bool isGrounded = false;
    float originalStepHeight = 0.5f;
    float originalCharHeight = 0.0f;
    PhysicsComponent();
    void release();
    ~PhysicsComponent();
};

#endif //PHYSICSCOMPONENT_H
