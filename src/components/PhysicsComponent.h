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
    ~PhysicsComponent();
};


inline PhysicsComponent::PhysicsComponent()
{
    controller = createPlayerController(
        PxVec3(DEFAULT_PLAYERPOS.x, DEFAULT_PLAYERPOS.y, DEFAULT_PLAYERPOS.z),
        0.4,
        0.7
    );
}

inline PhysicsComponent::~PhysicsComponent()
{
    controller->release();
}
#endif //PHYSICSCOMPONENT_H
