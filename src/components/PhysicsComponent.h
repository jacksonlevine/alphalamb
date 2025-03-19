//
// Created by jack on 3/16/2025.
//

#ifndef PHYSICSCOMPONENT_H
#define PHYSICSCOMPONENT_H

#include "../CollisionCage.h"

physx::PxController* getCont();


class PhysicsComponent {
public:
    CollisionCage collisionCage = {};
    physx::PxController* controller = getCont();
    bool isGrounded = false;
    float originalStepHeight = 0.5f;
    float originalCharHeight = 0.0f;
    //PhysicsComponent();
    ~PhysicsComponent();
    template<class Archive>
    void serialize(Archive& archive)
    {
        //deliberately empty
    }

    PhysicsComponent() = default;

    PhysicsComponent(const PhysicsComponent&) = delete;
    PhysicsComponent& operator=(const PhysicsComponent&) = delete;

    // Move constructor
    PhysicsComponent(PhysicsComponent&& other) noexcept {
        controller = other.controller;
        other.controller = nullptr;
        collisionCage.collider = other.collisionCage.collider;
        other.collisionCage.collider = nullptr;
    }

    // Move assignment operator
    PhysicsComponent& operator=(PhysicsComponent&& other) noexcept {
        if (this != &other) {

            controller = other.controller;
            other.controller = nullptr;
            collisionCage.collider = other.collisionCage.collider;
            other.collisionCage.collider = nullptr;
        }
        return *this;
    }
};

#endif //PHYSICSCOMPONENT_H
