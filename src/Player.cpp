//
// Created by jack on 1/6/2025.
//

#include "Player.h"

#include "PhysXStuff.h"
#include "world/gizmos/ParticlesGizmo.h"


void Player::update(const float deltaTime, World* world, ParticlesGizmo* particles)
{
    glm::vec3 displacement(0.0f, 0.0f, 0.0f);

    isGrounded = false;

    float walkmult = controls.sprint ? 8.0f : 5.2f;

    {
        auto locked = world->tryToGetReadLockOnDMs();
        if (locked != std::nullopt)
        {
            lastBlockStandingOn = (MaterialName)world->getLocked(IntTup(camera.transform.position.x , camera.transform.position.y-2 , camera.transform.position.z));
        }
    }

    if (controls.sprint && camera.transform.grounded)
    {
        if (footDustTimer > 0.07f)
        {
            footDustTimer = 0.0f;
            particles->particleBurst(camera.transform.position - glm::vec3(0.0, 0.9, 0.0), 1, lastBlockStandingOn, 0.5f, 0.05f);
        } else
        {
            footDustTimer += deltaTime;
        }
    }

    if(jetpackMode && controls.secondary1)
    {
        if(footDustTimer > 0.1)
        {
            particles->particleBurst(camera.transform.position - glm::vec3(0.0, 0.9, 0.0), 10, JETPACK_PARTICLE_BLOCK, 0.2f, 0.0f);
            footDustTimer = 0.0f;
        } else
        {
            footDustTimer += deltaTime;
        }
    }


    if (controls.secondary1)
    {
        jetpackMode = true;
    }

    if (jetpackMode)
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
        camera.transform.velocity.x = glm::clamp(camera.transform.velocity.x, -5000.0f * deltaTime, 5000.0f * deltaTime);
        camera.transform.velocity.z = glm::clamp(camera.transform.velocity.z, -5000.0f * deltaTime, 5000.0f * deltaTime);
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

    if (controls.secondary2)
    {
        hoverMode = !hoverMode;

        controls.secondary2 = false;
    }





    constexpr float JUMP_STRENGTH = 15.0f;
    constexpr float GRAVITY = 35.0f;
    constexpr float MAX_FALL_SPEED = 35.0f;

    // Apply gravity only if not grounded

    if (hoverMode)
    {
        camera.transform.velocity = glm::vec3(0.0f);
        jetpackMode = false;
        if (controls.sprint)
        {
            displacement -= glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)) * deltaTime * walkmult;
        }
        if (controls.jump)
        {
            displacement += glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)) * deltaTime * walkmult;
        }
    }else
    {

        //Have to do this I guess, or else it won't detect if we're on the ground consistently.
        displacement.y -= deltaTime * 3.0f;



        if (!camera.transform.grounded) {
            camera.transform.velocity.y -= GRAVITY * deltaTime;
            camera.transform.velocity.y = glm::max(camera.transform.velocity.y, -MAX_FALL_SPEED); // Clamp fall speed
        }

        // Handle jumping
        if ((camera.transform.grounded && controls.jump)) {
            camera.transform.velocity.y = JUMP_STRENGTH;
            controls.jump = false;
        }
    }





    displacement.y += camera.transform.velocity.y * deltaTime;
    displacement.x += camera.transform.velocity.x * deltaTime;
    displacement.z += camera.transform.velocity.z * deltaTime;

    PxControllerFilters filters;
    filters.mFilterFlags = PxQueryFlag::eSTATIC; // Only collides with static geometry

    // Move the character controller
    PxControllerCollisionFlags collisionFlags = controller->move(
        PxVec3(displacement.x, displacement.y, displacement.z),
        0.001f,
        deltaTime,
        filters
    );

    // Update grounded state
    camera.transform.grounded = (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN) || isGrounded;
    if (camera.transform.grounded)
    {
        jetpackMode = false;
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
    camera.transform.position.y = static_cast<float>(newPos.y + CAMERA_OFFSET);
    camera.transform.position.z = static_cast<float>(newPos.z);
}
Player::Player()
{
    controller = createPlayerController(
        PxVec3(DEFAULT_PLAYERPOS.x, DEFAULT_PLAYERPOS.y, DEFAULT_PLAYERPOS.z),
        0.4,
        0.7
    );
    PxVec3T controllerPosition = controller->getPosition();
    camera.transform.position = glm::vec3(controllerPosition.x, controllerPosition.y, controllerPosition.z);
    camera.updateWithYawPitch(0.0, 0.0);
    camera.updateProjection(1280, 1024, 90.0f);
}

Player::~Player()
{
    //std::cout << "Destructor called \n";
    controller->release();
}
