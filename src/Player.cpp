//
// Created by jack on 1/6/2025.
//

#include "Player.h"

#include "PhysXStuff.h"

void Player::update(const float deltaTime)
{
    glm::vec3 displacement(0.0f, 0.0f, 0.0f);
    auto mhr = ((MyControllerHitReport*)controller->getUserData());
    mhr->isGrounded = false;

    float walkmult = 5.0f;


    if (controls.secondary1)
    {
        mhr->jetpackMode = true;
    }

    if (mhr->jetpackMode)
    {
        camera.transform.velocity /= (1.0f + deltaTime);
        if (controls.secondary1) {
            camera.transform.velocity += camera.transform.up * 70.0f * deltaTime;
        }
        if (controls.forward) {
            camera.transform.velocity += glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.direction) * deltaTime * walkmult;
        }
        if (controls.backward) {
            camera.transform.velocity -= glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.direction) * deltaTime * walkmult;
        }
        if (controls.right) {
            camera.transform.velocity += glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.right) * deltaTime * walkmult;
        }
        if (controls.left) {
            camera.transform.velocity -= glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.right) * deltaTime * walkmult;
        }
    } else
    {
        if (controls.forward) {
            displacement += glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.direction) * deltaTime * walkmult;
        }
        if (controls.backward) {
            displacement -= glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.direction) * deltaTime * walkmult;
        }
        if (controls.right) {
            displacement += glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.right) * deltaTime * walkmult;
        }
        if (controls.left) {
            displacement -= glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.right) * deltaTime * walkmult;
        }
    }



    //Have to do this I guess, or else it won't detect if we're on the ground consistently.
    displacement.y -= 0.003f;



    constexpr float JUMP_STRENGTH = 15.0f;
    constexpr float GRAVITY = 25.0f;
    constexpr float MAX_FALL_SPEED = 35.0f;

    // Apply gravity only if not grounded

    if (!camera.transform.grounded) {
        camera.transform.velocity.y -= GRAVITY * deltaTime;
        camera.transform.velocity.y = glm::max(camera.transform.velocity.y, -MAX_FALL_SPEED); // Clamp fall speed
    }


    // Handle jumping
    if ((camera.transform.grounded && controls.jump) || FLY_MODE) {
        camera.transform.velocity.y = JUMP_STRENGTH;
        controls.jump = false;
    }

    displacement.y += camera.transform.velocity.y * deltaTime;
    displacement.x += camera.transform.velocity.x * deltaTime;
    displacement.z += camera.transform.velocity.z * deltaTime;

    // Move the character controller
    PxControllerCollisionFlags collisionFlags = controller->move(
        PxVec3(displacement.x, displacement.y, displacement.z),
        0.001f,
        deltaTime,
        PxControllerFilters()
    );

    // Update grounded state
    camera.transform.grounded = (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN) || mhr->isGrounded;
    if (camera.transform.grounded)
    {
        mhr->jetpackMode = false;
    }



    //std::cout << "Grounded: " << camera.transform.grounded << std::endl;
    // Reset vertical velocity if grounded
    if (camera.transform.grounded) {
        camera.transform.velocity.y = 0.0f;
        camera.transform.velocity.z = 0.0f;
        camera.transform.velocity.x = 0.0f;
    }

    // Update position
    PxExtendedVec3 newPos = controller->getPosition();
    camera.transform.position.x = static_cast<float>(newPos.x);
    camera.transform.position.y = static_cast<float>(newPos.y);
    camera.transform.position.z = static_cast<float>(newPos.z);
}
Player::Player()
{
    controller = createPlayerController(
        PxVec3(0, 200, 0),
        0.4,
        1.7
    );
    PxVec3T controllerPosition = controller->getPosition();
    camera.transform.position = glm::vec3(controllerPosition.x, controllerPosition.y, controllerPosition.z);
    camera.updateWithYawPitch(0.0, 0.0);
    camera.updateProjection(1280, 1024, 90.0f);
}

Player::~Player()
{
    controller->release();
}
