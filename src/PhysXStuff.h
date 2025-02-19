//
// Created by jack on 1/6/2025.
//

#ifndef PHYSXSTUFF_H
#define PHYSXSTUFF_H


#include "Transform.h"
#include "Controls.h"

using namespace physx;


extern PxDefaultAllocator gAllocator;
extern PxDefaultErrorCallback gErrorCallback;

extern PxFoundation* gFoundation;
extern PxPhysics* gPhysics;
extern PxDefaultCpuDispatcher* gDispatcher;
extern PxScene* gScene;
extern PxPvd* gPvd;
extern PxControllerManager* gControllerManager;

class SurfaceData
{
public:
    bool climbable;
    SurfaceData();
};

class MyControllerHitReport : public PxUserControllerHitReport {
public:
    bool isGrounded = false;
    bool jetpackMode = false;

    void onShapeHit(const PxControllerShapeHit& hit) override {
        // Get the shape we hit
        PxShape* hitShape = hit.shape;

        if (hit.worldNormal.y > 0.7f) {  // Adjust threshold as needed
            isGrounded = true;
           // printf("Ground hit detected!\n");
        }

        // Check if the shape has userData
        if (hitShape->userData != nullptr) {
            // Cast userData back to your custom struct
            SurfaceData* surfaceData = static_cast<SurfaceData*>(hitShape->userData);

            if (surfaceData->climbable) {
                printf("Touching a climbable surface!\n");
                // You are touching a climbable surface, do whatever logic you need here
            }
        }
    }

    void onControllerHit(const PxControllersHit& hit) override {}
    void onObstacleHit(const PxControllerObstacleHit& hit) override {}
};

void _initializePhysX();
PxRigidStatic* _createStaticMeshCollider(const PxVec3& position, const std::vector<PxVec3>& vertices, const std::vector<PxU32>& indices, bool climbable);
PxController* createPlayerController(const PxVec3& position, float radius, float height);
PxRigidDynamic* createPlayerKinematic(const PxVec3& position, float radius, float halfHeight);


PxRigidStatic* createStaticMeshCollider(const PxVec3& position, const std::vector<PxVec3>& vertices, const std::vector<PxU32>& indices);


PxRigidStatic* editStaticMeshCollider(PxRigidStatic* existing, const PxVec3& position, const std::vector<PxVec3>& vertices, const std::vector<PxU32>& indices);

void initializePhysX();

void destroyPhysXStuff();


inline static bool FLY_MODE = false;

inline void affectTransformAndPhysicsObjWithControls(jl::Transform& transform, Controls& controls, PxController* playerController, float deltaTime) {
    // 1. Create a displacement vector (this will be applied as the movement)
    glm::vec3 displacement(0.0f, 0.0f, 0.0f);


    // 3. Handle player movement (left, right, forward, backward)
    float walkmult = 9.8f;
    if (controls.forward) {
        displacement += normalize(glm::vec3(1.0f, 0.0f, 1.0f) * transform.direction) * deltaTime * walkmult;
    }
    if (controls.backward) {
        displacement -= normalize(glm::vec3(1.0f, 0.0f, 1.0f) * transform.direction) * deltaTime * walkmult;
    }
    if (controls.right) {
        displacement += normalize(glm::vec3(1.0f, 0.0f, 1.0f) * transform.right) * deltaTime * walkmult;
    }
    if (controls.left) {
        displacement -= normalize(glm::vec3(1.0f, 0.0f, 1.0f) * transform.right) * deltaTime * walkmult;
    }


    // Jumping mechanics
    const float JUMP_STRENGTH = 30.0f;
    const float GRAVITY = 35.0f;
    const float MAX_FALL_SPEED = 50.0f;


    if(!FLY_MODE)
    {
        // Apply gravity
        transform.velocity.y -= GRAVITY * deltaTime;

    }


    // Limit falling speed
    transform.velocity.y = glm::max(transform.velocity.y, -MAX_FALL_SPEED);

    // Jumping
    if ((transform.grounded && controls.jump) || FLY_MODE)
    {
        //std::cout << "Jump \n";
        transform.velocity.y = JUMP_STRENGTH;  // Set to a fixed jump velocity
        controls.jump = false;
    }

    // Apply velocity to displacement
    displacement.y += transform.velocity.y * deltaTime;


    PxControllerCollisionFlags collisionFlags = playerController->move(
        PxVec3(displacement.x, displacement.y, displacement.z),
        0.001f,
        deltaTime,
        PxControllerFilters()
    );

    transform.grounded = (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN);


    // 5. Update transform position from playerController's current position
    PxExtendedVec3 newPos = playerController->getPosition();
    transform.position.x = static_cast<float>(newPos.x);
    transform.position.y = static_cast<float>(newPos.y);
    transform.position.z = static_cast<float>(newPos.z);
}

#endif //PHYSXSTUFF_H
