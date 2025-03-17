//
// Created by jack on 3/16/2025.
//
#include "PhysicsComponent.h"
#include "../PhysXStuff.h"
#include "../Player.h"

PhysicsComponent::PhysicsComponent()
{
    controller = createPlayerController(
        PxVec3(DEFAULT_PLAYERPOS.x, DEFAULT_PLAYERPOS.y, DEFAULT_PLAYERPOS.z),
        0.4,
        0.7
    );
}

void PhysicsComponent::release()
{
    controller->release();
    collisionCage.collider->release();
}

PhysicsComponent::~PhysicsComponent()
{
    //gScene->removeActor(controller->getActor())
    //Ah motherfucker for fucks sake. I can't get it to work releasing it in the destructor, I'm just gonna make a method to do it
    // std::cout << "RELEASE Number of controllers before release: " << gControllerManager->getNbControllers();
    // controller->release();
    // std::cout << "RELEASE Number of controllers: " << gControllerManager->getNbControllers();
}
