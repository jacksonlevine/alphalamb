//
// Created by jack on 3/16/2025.
//

#ifndef MOVEMENTCOMPONENT_H
#define MOVEMENTCOMPONENT_H

#include "../PrecompHeader.h"

struct MovementComponent {
    // Jetpack related
    bool jetpackMode = false;
    bool hoverMode = false;
    ALuint jetpackSource = 0;

    ALuint footstepSource = 0;

    // Dash related
    int stamCount = 3;
    bool dashing = false;
    float dashtimer = 0.0f;
    float dashrebuild = 0.0f;

    // Sliding related
    bool isSliding = false;
    float slideTimer = 0.0f;
    const float slideDuration = 1.5f;
    bool slidThisDash = false;
    bool crouchOverride = false;

    // Ledge grab related
    glm::vec3 ledgePosition = {};
    glm::vec3 ledgeNormal = {};
    glm::vec3 climbStartPosition = {};
    bool isLedgeGrabbing = false;
    bool isClimbingUp = false;
    float climbUpTimer = 0.0f;
    float ledgeGrabCooldown = 0.0f;
    float footstepTimer = 0.0f;
    float footstepInterval = 0.5f;

    float crouchDegree = 0.0f;

    int soundSeriesIndexer = 0;

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(stamCount, ledgePosition, ledgeNormal, climbStartPosition, isLedgeGrabbing, isClimbingUp, climbUpTimer, ledgeGrabCooldown);
    }
};

#endif //MOVEMENTCOMPONENT_H
