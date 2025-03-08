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

#endif //PHYSXSTUFF_H
