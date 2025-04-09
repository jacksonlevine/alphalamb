//
// Created by jack on 1/6/2025.
//

#include "Player.h"

#include "OpenALStuff.h"
#include "PhysXStuff.h"
#include "components/InventoryComponent.h"
#include "components/ParticleEffectComponent.h"
#include "world/FootstepSounds.h"
#include "world/gizmos/ParticlesGizmo.h"

// void Player::update(const float deltaTime, World* world, ParticlesGizmo* particles)
// {
//
//
//
// }





void PlayerUpdate(float deltaTime, World* world, ParticlesGizmo* particles, RenderComponent& renderComponent,
    PhysicsComponent& physicsComponent, MovementComponent& movementComponent, Controls& controls, jl::Camera & camera, ParticleEffectComponent & particleComponent, InventoryComponent& inventory)
{


    if (controls.swimming) {
        // Totally different player movement logic for swimming
        float swimSpeed = 8.0f; // Base swimming speed

        // Zero out gravity effects when swimming
        camera.transform.velocity /= (1.0f + deltaTime * 2.0f); // Higher water resistance

        // Controls for swimming movement - 3D movement in water
        if (controls.forward) {
            camera.transform.velocity += camera.transform.direction * deltaTime * swimSpeed;
        }
        if (controls.backward) {
            camera.transform.velocity -= camera.transform.direction * deltaTime * swimSpeed;
        }
        if (controls.right) {
            camera.transform.velocity += camera.transform.right * deltaTime * swimSpeed;
        }
        if (controls.left) {
            camera.transform.velocity -= camera.transform.right * deltaTime * swimSpeed;
        }
        if (controls.jump) {
            camera.transform.velocity += glm::vec3(0.0f, 1.0f, 0.0f) * deltaTime * swimSpeed;
        }
        if (controls.crouch) {
            camera.transform.velocity -= glm::vec3(0.0f, 1.0f, 0.0f) * deltaTime * swimSpeed;
        }

        // Apply natural buoyancy - gentle upward drift
        camera.transform.velocity += glm::vec3(0.0f, 0.5f, 0.0f) * deltaTime;

        // Apply velocity-based movement
        glm::vec3 displacement = camera.transform.velocity * deltaTime;

        // Move controller with swim physics
        PxControllerFilters filters;
        filters.mFilterFlags = PxQueryFlag::eSTATIC;
        physicsComponent.controller->move(
            PxVec3(displacement.x, displacement.y, displacement.z),
            0.001f,
            deltaTime,
            filters
        );

        // Update position
        PxExtendedVec3 newPos = physicsComponent.controller->getPosition();
        camera.transform.position.x = static_cast<float>(newPos.x);
        camera.transform.position.y = static_cast<float>(newPos.y + CAMERA_OFFSET - (movementComponent.crouchOverride ? 1.0f: 0.0f));
        camera.transform.position.z = static_cast<float>(newPos.z);

        // particleComponent.footDustTimer += deltaTime;
        // if (particleComponent.footDustTimer > 0.3f) {
        //     particles->particleBurst(camera.transform.position, 3, WATER, 0.8f, 0.1f);
        //     particleComponent.footDustTimer = 0.0f;
        // }

        return; // Skip the rest of the normal movement logic
    }

    //std::cout << "PlayerUpdate with seed " << world->worldGenMethod->getSeed() << std::endl;

    auto & jetpackSource = movementComponent.jetpackSource;
    auto & isGrounded = physicsComponent.isGrounded;
    auto & stamCount = movementComponent.stamCount;
    auto & dashrebuild = movementComponent.dashrebuild;
    auto & crouchOverride = movementComponent.crouchOverride;
    auto & originalCharHeight = physicsComponent.originalCharHeight;
    auto & controller = physicsComponent.controller;
    auto & dashing = movementComponent.dashing;
    auto & dashtimer = movementComponent.dashtimer;
    auto & slidThisDash = movementComponent.slidThisDash;
    auto & isSliding = movementComponent.isSliding;
    auto & lastBlockStandingOn = particleComponent.lastBlockStandingOn;
    auto & slideTimer = movementComponent.slideTimer;
    auto & slideDuration = movementComponent.slideDuration;
    auto & originalStepHeight = physicsComponent.originalStepHeight;
    auto & isClimbingUp = movementComponent.isClimbingUp;
    auto & isLedgeGrabbing = movementComponent.isLedgeGrabbing;
    auto & footDustTimer = particleComponent.footDustTimer;
    auto & ledgeGrabCooldown = movementComponent.ledgeGrabCooldown;
    auto & jetpackMode = movementComponent.jetpackMode;
    auto & hoverMode = movementComponent.hoverMode;
    auto & ledgeNormal = movementComponent.ledgeNormal;
    auto & ledgePosition = movementComponent.ledgePosition;
    auto & climbUpTimer = movementComponent.climbUpTimer;
    auto & climbStartPosition = movementComponent.climbStartPosition;



    controller->setPosition(PxExtendedVec3(
        camera.transform.position.x,
        camera.transform.position.y - CAMERA_OFFSET + (crouchOverride ? 1.0f: 0.0f),
        camera.transform.position.z
        ));



    if (jetpackSource == 0)
    {
        jetpackSource = makeSource(camera.transform.position);
        setSourceBuffer(sounds.at((int)SoundBuffers::JETPACK), jetpackSource);
        alSourcei(jetpackSource, AL_LOOPING, AL_TRUE);

        movementComponent.footstepSource = makeSource(camera.transform.position - glm::vec3(0,1,0));

        alSourcei(jetpackSource, AL_LOOPING, AL_FALSE);
    }



    if(controls.anyMovement())
    {
        if(controls.sprint)
        {
            movementComponent.footstepInterval = 0.35f;
        } else
        {
            movementComponent.footstepInterval = 0.5f;
        }
        if(auto b =  getFootstepSound(lastBlockStandingOn); b != std::nullopt)
        {
            if(movementComponent.footstepTimer < movementComponent.footstepInterval)
            {
                movementComponent.footstepTimer += deltaTime;
            } else
            {
                movementComponent.footstepTimer = 0.0f;

                auto & stepSounds = getBufferSeries(fromMaterial(static_cast<MaterialName>(lastBlockStandingOn & BLOCK_ID_BITS)));
                if(!stepSounds.empty())
                {
                    if(movementComponent.soundSeriesIndexer < stepSounds.size() && movementComponent.soundSeriesIndexer >= 0)
                    {
                        alSourceStop(movementComponent.footstepSource);
                        playBufferFromSource(sounds.at((int)stepSounds[movementComponent.soundSeriesIndexer]), movementComponent.footstepSource);
                    }
                    movementComponent.soundSeriesIndexer  = (movementComponent.soundSeriesIndexer + 1) % stepSounds.size();
                }


            }
        }
    }

    glm::vec3 displacement(0.0f, 0.0f, 0.0f);
    isGrounded = false;

    if (stamCount < 3)
    {
        if (dashrebuild < 3.0f)
        {
            dashrebuild += deltaTime;
        } else
        {
            dashrebuild = 0.0f;
            stamCount++;
        }
    }

    if (crouchOverride)
    {
        PxCapsuleController* capsuleController = static_cast<PxCapsuleController*>(controller);
        originalCharHeight = capsuleController->getHeight();
        capsuleController->resize(0.001f);
    } else if(!controls.crouch)
    {
        PxCapsuleController* capsuleController = static_cast<PxCapsuleController*>(controller);
        capsuleController->resize(originalCharHeight);
    }

    if (controls.sprint && !dashing)
    {
        if (stamCount > 0 && dashtimer <= 0.0f)
        {
            dashing = true;
            slidThisDash = false;
            stamCount-= 1;
            dashtimer = 2.0f;
            dashrebuild = 0.0f;
        }
    }

    if(!controls.sprint && dashing)
    {
        dashing = false;
        dashtimer = 0.0f;
        dashrebuild = 0.0f;
    }

    if (dashtimer > 0.0f)
    {
        dashtimer -= deltaTime;
    } else
    {
        controls.sprint = false;
        dashing = false;
    }




    // Handle sliding logic
    if (dashing && crouchOverride && !isSliding && !slidThisDash)
    {
        // Start sliding
        isSliding = true;
        slideTimer = slideDuration;
        slidThisDash = true;
        // Store original step height and set to lower height while sliding
        originalStepHeight = controller->getStepOffset();
        controller->setStepOffset(1.5f); //Slide up blocks
        isClimbingUp = false;
        isLedgeGrabbing = false;

    }

    // Update sliding state
    if (isSliding)
    {
        isClimbingUp = false;
        isLedgeGrabbing = false;
        // Update slide timer
        slideTimer -= deltaTime;

        // Handle slide dust particles
        if (camera.transform.grounded)
        {
            if (footDustTimer > 0.05f) // More frequent particles during slide
            {
                footDustTimer = 0.0f;
                particles->particleBurst(camera.transform.position - glm::vec3(0.0, 0.9, 0.0), 2, lastBlockStandingOn, 0.5f, 0.05f);
            }
            else
            {
                footDustTimer += deltaTime;
            }
        }

        // End slide if timer runs out
        if (slideTimer <= 0.0f)
        {
            isSliding = false;
            slideTimer = 0.0f;

            // Restore original step height
            controller->setStepOffset(originalStepHeight);
            PxCapsuleController* capsuleController = static_cast<PxCapsuleController*>(controller);
            capsuleController->resize(originalCharHeight);
        }

        // If player releases crouch during slide, end it early
        if (!crouchOverride)
        {
            isSliding = false;
            slideTimer = 0.0f;
            controller->setStepOffset(originalStepHeight);
            PxCapsuleController* capsuleController = static_cast<PxCapsuleController*>(controller);
            capsuleController->resize(originalCharHeight);
        }
    }

    float walkmult = 4.0f;
    if (dashing && !isSliding) {
        walkmult = 9.0f;
    } else if (isSliding) {
        // Increase speed at start of slide, then gradually reduce
        float slideSpeedMultiplier = 1.0f * (slideTimer / slideDuration);
        walkmult = 9.0f * (1.0f + slideSpeedMultiplier);
    }

    // Check what block we're standing on...
    {
        auto locked = world->tryToGetReadLockOnDMs();
        if (locked != std::nullopt)
        {
            lastBlockStandingOn = (MaterialName)world->getLocked(IntTup(camera.transform.position.x, camera.transform.position.y-2, camera.transform.position.z));
            if (world->getLocked(IntTup(std::floor(camera.transform.position.x), std::floor(camera.transform.position.y), std::floor(camera.transform.position.z))) != AIR)
            {
                crouchOverride = true;
            } else if (world->getLocked(IntTup(std::floor(camera.transform.position.x), std::floor(camera.transform.position.y + 1), std::floor(camera.transform.position.z))) == AIR)
            {
                crouchOverride = controls.crouch;
            }
        }
    }

    // Handle sprint dust particles...
    if (dashing && camera.transform.grounded && !isSliding)
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

    static const float climbUpDuration = 0.5f; // Time to climb up in seconds

    // Update cooldown
    if (ledgeGrabCooldown > 0.0f) {
        ledgeGrabCooldown -= deltaTime;
    }

    // Ledge detection and handling
    if (!isLedgeGrabbing && !isClimbingUp && !camera.transform.grounded && !jetpackMode && !hoverMode && ledgeGrabCooldown <= 0.0f && slideTimer <= 0.0f && !isSliding && !controls.crouch)
    {
        // Check if we're falling
        if (camera.transform.velocity.y < 0)
        {
            // Cast a ray forward to detect a ledge
            PxRaycastBuffer raycastResult;
            PxVec3 rayOrigin(camera.transform.position.x, camera.transform.position.y, camera.transform.position.z);
            PxVec3 rayDir(camera.transform.direction.x, 0, camera.transform.direction.z);
            rayDir.normalize();

            // Setup query filter data - only hit static objects (word0 = 1), not controllers (word0 = 2)
            PxQueryFilterData filterData;
            filterData.flags = PxQueryFlag::eSTATIC; // Only hit static geometry
            filterData.data.word0 = 1; // Only hit objects with this filter bit

            // Cast ray forward to detect a wall with filter
            if (gScene->raycast(rayOrigin, rayDir, 1.2f, raycastResult, PxHitFlag::eDEFAULT, filterData))
            {
                // Save normal for proper positioning
                PxVec3 normal = raycastResult.block.normal;

                // Now check if there's a ledge by casting a ray from above the hit point
                PxRaycastBuffer ledgeRayResult;

                // Position the ledge ray at the block face we hit, offset back a bit along normal
                PxVec3 hitPos = raycastResult.block.position;

                // Start ray above where we hit the wall
                PxVec3 ledgeRayOrigin = hitPos + PxVec3(0, 1.5f, 0) + rayDir * 0.3;
                PxVec3 ledgeRayDir(0, -1, 0);

                // Debug rays if needed
                // std::cout << "Ray origin: " << ledgeRayOrigin.x << ", " << ledgeRayOrigin.y << ", " << ledgeRayOrigin.z << std::endl;

                if (gScene->raycast(ledgeRayOrigin, ledgeRayDir, 2.0f, ledgeRayResult))
                {
                    // Found a ledge, check if it's within grab range (height)
                    float ledgeHeight = ledgeRayResult.block.position.y;

                    // Ensure the ledge is at a valid height for grabbing
                    if (ledgeHeight > camera.transform.position.y &&
                        ledgeHeight < camera.transform.position.y + 1.0f)
                    {
                        // Check if the space above the ledge is clear (not a block)
                        // We need to make sure there's room to climb up
                        IntTup ledgeBlockPos(
                            floor(ledgeRayResult.block.position.x),
                            floor(ledgeRayResult.block.position.y) + 1, // Check block above ledge
                            floor(ledgeRayResult.block.position.z)
                        );
                        IntTup ledgeBlockPos2(
                            floor(ledgeRayResult.block.position.x),
                            floor(ledgeRayResult.block.position.y) + 2, // Check block above ledge2
                            floor(ledgeRayResult.block.position.z)
                        );

                        bool isSpaceClear = true;
                        {
                            auto locked = world->tryToGetReadLockOnDMs();
                            if (locked != std::nullopt)
                            {
                                int blockType = world->getLocked(ledgeBlockPos);
                                int blockType2 = world->getLocked(ledgeBlockPos2);
                                isSpaceClear = ((blockType == AIR) && (blockType2 == AIR));
                            }
                        }

                        if (isSpaceClear)
                        {
                            // We can grab this ledge!
                            isLedgeGrabbing = true;
                            ledgePosition = glm::vec3(ledgeRayResult.block.position.x,
                                                    ledgeHeight,
                                                    ledgeRayResult.block.position.z);

                            // Store normal for better positioning
                            ledgeNormal = glm::vec3(normal.x, normal.y, normal.z);

                            // Position the player at the ledge - careful with voxel coordinates
                            // Position slightly away from the ledge along the normal
                            glm::vec3 grabPosition = ledgePosition + (ledgeNormal * 0.45f);
                            grabPosition.y = ledgeHeight - 0.6f; // Position hands at ledge height

                            // Snap to grid to prevent getting stuck in blocks
                            // Move slightly away from any block boundaries
                            grabPosition.x = floor(grabPosition.x) + 0.5f + (ledgeNormal.x * 0.2f);
                            grabPosition.z = floor(grabPosition.z) + 0.5f + (ledgeNormal.z * 0.2f);


                            bool spotclear = true;
                            {
                                auto locked = world->tryToGetReadLockOnDMs();
                                if (locked != std::nullopt)
                                {
                                    int blockType = world->getLocked(IntTup(
                                        std::floor(grabPosition.x),
                                        std::floor(grabPosition.y-CAMERA_OFFSET),
                                        std::floor(grabPosition.z)));
                                    int blockType2 = world->getLocked(IntTup(
                                        std::floor(grabPosition.x),
                                        std::floor(grabPosition.y-CAMERA_OFFSET)+1,
                                        std::floor(grabPosition.z)));
                                    spotclear = ((blockType == AIR) && (blockType2 == AIR));
                                }
                            }

                            if (spotclear)
                            {
                                // Update controller position
                                controller->setPosition(PxExtendedVec3(grabPosition.x, grabPosition.y - CAMERA_OFFSET, grabPosition.z));

                                // Zero out velocity
                                camera.transform.velocity = glm::vec3(0.0f);

                                // Play grab sound if you have one
                                // playSound(sounds.at((int)SoundBuffers::LEDGE_GRAB));
                            }

                        }
                    }
                }
            }
        }
    }
    //std::cout << "isledgegrabbing" << isLedgeGrabbing << std::endl;
    //std::cout << "isclimbingup: " << isClimbingUp << std::endl;
    //std::cout << "issliding: " << isSliding << std::endl;
    //std::cout << "isledgegrabbing: " << isLedgeGrabbing << std::endl;
    // Handle ledge grabbing state
    if (isLedgeGrabbing && !isClimbingUp && !isSliding)
    {
        // Zero out velocity and displacement while hanging
        camera.transform.velocity = glm::vec3(0.0f);
        displacement = glm::vec3(0.0f);

        // Start climbing if jump is pressed
        if ((controls.jump || controls.forward) && !isSliding)
        {
            isClimbingUp = true;
            isLedgeGrabbing = false;
            climbUpTimer = 0.0f;
            climbStartPosition = camera.transform.position;
            controls.jump = false; // Consume the jump input
        }

        // Let go if down is pressed
        if (controls.backward)
        {
            isLedgeGrabbing = false;
            camera.transform.velocity.y = -0.5f; // Slight downward velocity
            ledgeGrabCooldown = 0.5f; // Prevent immediate grab after letting go
        }
    }

    // Handle climbing up
    if (isClimbingUp && !isSliding)
    {
        // Advance timer
        climbUpTimer += deltaTime;
        float t = climbUpTimer / climbUpDuration;

        if (t >= 1.0f)
        {
            // Finished climbing
            isClimbingUp = false;
            ledgeGrabCooldown = 0.5f; // Prevent immediate grab after climbing

            // Position on top of ledge with safety offset to prevent getting stuck in blocks
            glm::vec3 finalPos = ledgePosition;

            // Make sure we end up on the safe side of the ledge
            finalPos += ledgeNormal * 0.75f; // Good safety margin

            // Snap to grid center for X and Z to avoid wall edges
            finalPos.x = floor(finalPos.x) + 0.5f + (ledgeNormal.x * 0.2f);
            finalPos.z = floor(finalPos.z) + 0.5f + (ledgeNormal.z * 0.2f);

            // Set proper Y position
            finalPos.y = ceil(ledgePosition.y) + 1.5f; // Slightly above block


            bool spotclear = true;
            {
                auto locked = world->tryToGetReadLockOnDMs();
                if (locked != std::nullopt)
                {
                    int blockType = world->getLocked(IntTup(
                        std::floor(finalPos.x),
                        std::floor(finalPos.y-CAMERA_OFFSET),
                        std::floor(finalPos.z)));
                    int blockType2 = world->getLocked(IntTup(
                        std::floor(finalPos.x),
                        std::floor(finalPos.y-CAMERA_OFFSET)+1,
                        std::floor(finalPos.z)));
                    spotclear = ((blockType == AIR) && (blockType2 == AIR));
                }
            }

            if (spotclear)
            {
                // Update controller position
                controller->setPosition(PxExtendedVec3(finalPos.x, finalPos.y - CAMERA_OFFSET, finalPos.z));

                // Update camera position
                camera.transform.position = finalPos;
            }

        }
        else
        {
            // Climbing animation - interpolate between start position and end position
            glm::vec3 targetPos = ledgePosition;

            // Make sure target position is safe
            targetPos += ledgeNormal * 0.75f;
            targetPos.x = floor(targetPos.x) + 0.5f + (ledgeNormal.x * 0.2f);
            targetPos.z = floor(targetPos.z) + 0.5f + (ledgeNormal.z * 0.2f);
            targetPos.y = ceil(ledgePosition.y) + 1.5f;

            // Use smooth easing with a more natural curve
            // Start slow, accelerate in middle, end slow
            float easedT = t * t * (3.0f - 2.0f * t); // Smooth step function

            // Add a slight curve to the climb path
            glm::vec3 newPos;

            // First half of animation: more upward than forward
            if (t < 0.5f) {
                float adjustedT = easedT * 2.0f; // Scale t to 0-1 range for first half
                glm::vec3 midPoint = climbStartPosition;
                midPoint.y = (targetPos.y + climbStartPosition.y) * 0.5f;
                midPoint += ledgeNormal * 0.2f;
                newPos = glm::mix(climbStartPosition, midPoint, adjustedT);
            }
            // Second half: more forward than upward
            else {
                float adjustedT = (easedT - 0.5f) * 2.0f; // Scale t to 0-1 range for second half
                glm::vec3 midPoint = climbStartPosition;
                midPoint.y = (targetPos.y + climbStartPosition.y) * 0.5f;
                midPoint += ledgeNormal * 0.2f;
                newPos = glm::mix(midPoint, targetPos, adjustedT);
            }

            bool spotclear = false;
            {
                auto locked = world->tryToGetReadLockOnDMs();
                if (locked != std::nullopt)
                {
                    int blockType = world->getLocked(IntTup(
                        std::floor(newPos.x),
                        std::floor(newPos.y-CAMERA_OFFSET),
                        std::floor(newPos.z)));
                    int blockType2 = world->getLocked(IntTup(
                        std::floor(newPos.x),
                        std::floor(newPos.y-CAMERA_OFFSET)+1,
                        std::floor(newPos.z)));
                    spotclear = ((blockType == AIR) && (blockType2 == AIR));
                }
            }
            if (spotclear)
            {
                // Update controller position
                controller->setPosition(PxExtendedVec3(newPos.x, newPos.y - CAMERA_OFFSET, newPos.z));

                // Update camera position directly
                camera.transform.position = newPos;

                // Zero out velocity and displacement during climb
                camera.transform.velocity = glm::vec3(0.0f);
                displacement = glm::vec3(0.0f);
            } else
            {
                isClimbingUp = false;
            }

        }
    }

    // Only process normal movement if not ledge grabbing or climbing
    if (!isLedgeGrabbing && !isClimbingUp)
    {

        ALint jpSourceState;
        alGetSourcei(jetpackSource, AL_SOURCE_STATE, &jpSourceState);

        if(jetpackMode && controls.secondary1)
        {
            setSourcePosVel(jetpackSource, camera.transform.position - glm::vec3(0,1,0), camera.transform.velocity);

            if (jpSourceState != AL_PLAYING)
            {
                alSourcePlay(jetpackSource);
            }

            if(footDustTimer > 0.1)
            {
                particles->particleBurst(camera.transform.position - glm::vec3(0.0, 0.9, 0.0), 10, JETPACK_PARTICLE_BLOCK, 0.2f, 0.0f);
                footDustTimer = 0.0f;
            } else
            {
                footDustTimer += deltaTime;
            }
        } else
        {
            if (jpSourceState == AL_PLAYING)
            {
                alSourceStop(jetpackSource);
            }
        }

        if (controls.secondary1)
        {
            for (auto& item : inventory.inventory.getEquippedItems()) {
                if (item.block == ItemName::JETPACK)
                {
                    jetpackMode = true;
                }
            }
        }

        if (jetpackMode)
        {
            camera.transform.velocity /= (1.0f + deltaTime);
            if (controls.secondary1) {
                camera.transform.velocity += camera.transform.up * 90.0f * deltaTime;
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
            camera.transform.velocity.x = glm::clamp(camera.transform.velocity.x, -8000.0f * deltaTime, 8000.0f * deltaTime);
            camera.transform.velocity.z = glm::clamp(camera.transform.velocity.z, -8000.0f * deltaTime, 8000.0f * deltaTime);
        } else
        {
            if (isSliding)
            {
                // During sliding, prioritize forward momentum and limit turning
                if (controls.forward)
                {
                    displacement += glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.direction) * deltaTime * walkmult;
                }

                // Allow slight steering during slide (reduced effect)
                if (controls.right)
                {
                    displacement += glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.right) * deltaTime * walkmult * 0.3f;
                }
                if (controls.left)
                {
                    displacement -= glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * camera.transform.right) * deltaTime * walkmult * 0.3f;
                }

                // Ignore backward input during slide
                // controls.backward is not processed during sliding
            }
            else
            {
                // Normal movement when not sliding
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
        }

        if (controls.secondary2)
        {
            hoverMode = !hoverMode;
            controls.secondary2 = false;
        }

        constexpr float JUMP_STRENGTH = 15.0f;
        constexpr float GRAVITY = 52.0f;
        constexpr float MAX_FALL_SPEED = 35.0f;

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
        } else
        {
            //Have to do this I guess, or else it won't detect if we're on the ground consistently.

            auto amount = deltaTime * 3.0f;
            displacement.y -= std::min(amount, 0.01f);

            if (!camera.transform.grounded) {
                camera.transform.velocity.y -= GRAVITY * deltaTime;
                camera.transform.velocity.y = glm::max(camera.transform.velocity.y, -MAX_FALL_SPEED); // Clamp fall speed
            }

            // Handle jumping - don't allow jumping during sliding
            if ((camera.transform.grounded && controls.jump && !isSliding)) {
                camera.transform.velocity.y = JUMP_STRENGTH;
                controls.jump = false;
            }
        }
    }

    if (!isClimbingUp)
    {
        displacement.y += camera.transform.velocity.y * deltaTime;
        displacement.x += camera.transform.velocity.x * deltaTime;
        displacement.z += camera.transform.velocity.z * deltaTime;

        PxControllerFilters filters;
        filters.mFilterFlags = PxQueryFlag::eSTATIC;

        if (isSliding) {
            controller->setStepOffset(1.5f);
        }

        PxControllerCollisionFlags collisionFlags = controller->move(
            PxVec3(displacement.x, displacement.y, displacement.z),
            0.001f,
            deltaTime,
            filters
        );

        camera.transform.grounded = (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN) || isGrounded;
        if (camera.transform.grounded)
        {
            jetpackMode = false;
            // Also reset ledge grabbing state if we somehow got grounded
            isLedgeGrabbing = false;
            isClimbingUp = false;
        }

        // // If we're sliding and hit a wall, end the slide
        // if (isSliding && (
        //     (collisionFlags & PxControllerCollisionFlag::eCOLLISION_SIDES) ))
        // {
        //     isSliding = false;
        //     slideTimer = 0.0f;
        //     controller->setStepOffset(originalStepHeight);
        // }

        // Reset vertical velocity if grounded
        if (camera.transform.grounded) {
            camera.transform.velocity.y = 0.0f;
            if (!isSliding) {
                // Only zero out horizontal velocity if not sliding
                camera.transform.velocity.z = 0.0f;
                camera.transform.velocity.x = 0.0f;
            } else {
                // Gradually reduce horizontal velocity during slide
                camera.transform.velocity.z *= 0.98f;
                camera.transform.velocity.x *= 0.98f;
            }
        }

        // Update position
        PxExtendedVec3 newPos = controller->getPosition();
        camera.transform.position.x = static_cast<float>(newPos.x);
        camera.transform.position.y = static_cast<float>(newPos.y + CAMERA_OFFSET - (crouchOverride ? 1.0f: 0.0f));
        camera.transform.position.z = static_cast<float>(newPos.z);
    }
}
//
// Player::Player()
// {
//     controller = createPlayerController(
//         PxVec3(DEFAULT_PLAYERPOS.x, DEFAULT_PLAYERPOS.y, DEFAULT_PLAYERPOS.z),
//         0.4,
//         0.7
//     );
//     PxVec3T controllerPosition = controller->getPosition();
//     camera.transform.position = glm::vec3(controllerPosition.x, controllerPosition.y, controllerPosition.z);
//     camera.updateWithYawPitch(0.0, 0.0);
//     camera.updateProjection(1280, 1024, 90.0f);
// }
//
// Player::~Player()
// {
//     //std::cout << "Destructor called \n";
//     controller->release();
// }
