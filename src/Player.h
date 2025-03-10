
//
// Created by jack on 1/6/2025.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "PrecompHeader.h"
#include "Inventory.h"

#include "Camera.h"
#include "CollisionCage.h"
#include "Controls.h"
#include "BillboardInstanceShader.h"

inline static const PxExtendedVec3 DEFAULT_PLAYERPOS = PxExtendedVec3(0, 200, 0);
inline static const float CAMERA_OFFSET = 0.8f;
class MyControllerHitReport;
class ParticlesGizmo;

struct Player {
    jl::Camera camera = {};

    ChunkGLInfo handledBlockMeshInfo = {};
    Billboard billboard = {};
    CollisionCage collisionCage = {};
    AnimationState animation_state = {};
    physx::PxController* controller = nullptr;
    Controls controls = {};
    void update(float deltaTime, World* world, ParticlesGizmo* particles);
    MaterialName lastHeldBlock = AIR;
    MaterialName currentHeldBlock = WOOD_PLANKS;
    float footDustTimer = 0.0f;
    MaterialName lastBlockStandingOn = AIR;
    bool isGrounded = false;
    bool jetpackMode = false;
    bool hoverMode = false;
    ALuint jetpackSource = 0;
    Inventory inventory = {};
    std::weak_ptr<boost::asio::ip::tcp::socket> socket;
    bool receivedWorld = false;
    int stamCount = 3;
    bool dashing = false;
    float dashtimer = 0.0f;
    float dashrebuild = 0.0f;
    Player();
    ~Player();
private:
    // Ledge grab variables
    glm::vec3 ledgePosition;
    glm::vec3 ledgeNormal; // Store the normal for better positioning
    glm::vec3 climbStartPosition;
    bool isLedgeGrabbing = false;
    bool isClimbingUp = false;
    float climbUpTimer = 0.0f;
    float ledgeGrabCooldown = 0.0f; // Cooldown to prevent immediately grabbing again

};



#endif //PLAYER_H
