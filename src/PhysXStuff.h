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

enum class CollisionGroup
{
    GROUP_PLAYER   = 1 << 0,
    GROUP_WORLD    = 1 << 1,
    GROUP_PARTICLE = 1 << 2,
    GROUP_ANIMAL   = 1 << 3,
    GROUP_LOOT     = 1 << 4,
};
inline void setCollisionFilter(PxShape* shape, uint32_t group, uint32_t whaticollidewith)
{
    PxFilterData fd;
    fd.word0 = group;
    fd.word1 = whaticollidewith;

    shape->setSimulationFilterData(fd);
    shape->setQueryFilterData(fd);
}



class MyContFiltCallback : public PxQueryFilterCallback
{
public:
    PxQueryHitType::Enum preFilter(
        const PxFilterData& filterData,
        const PxShape* shape,
        const PxRigidActor* actor,
        PxHitFlags& queryFlags) override
    {
        // Get the shape's filter data
        PxFilterData shapeFilter = shape->getSimulationFilterData();

        if ((filterData.word0 & shapeFilter.word1) == 0 &&
            (shapeFilter.word0 & filterData.word1) == 0)
        {
            return PxQueryHitType::eNONE; // No collision - filter out
        }

        return PxQueryHitType::eBLOCK; // Allow collision
    }

    PxQueryHitType::Enum postFilter(
        const PxFilterData& filterData,
        const PxQueryHit& hit,
        const PxShape* shape,
        const PxRigidActor* actor) override
    {
        // Get the shape's filter data
        PxFilterData shapeFilter = shape->getSimulationFilterData();

        if ((filterData.word0 & shapeFilter.word1) == 0 &&
            (shapeFilter.word0 & filterData.word1) == 0)
        {
            return PxQueryHitType::eNONE; // No collision - filter out
        }

        return PxQueryHitType::eBLOCK; // Allow collision
    }
};
class MyBehaviorCallback : public PxControllerBehaviorCallback
{
public:
    PxFilterData playerFilterData;

    PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor) override
    {
        PxFilterData shapeFilter = shape.getSimulationFilterData();

        if ((playerFilterData.word0 & shapeFilter.word1) == 0 &&
            (shapeFilter.word0 & playerFilterData.word1) == 0)
        {
            return PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;
        }

        return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT; // Allow stepping
    }

    PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) override
    {
        return PxControllerBehaviorFlags(0); // No special behavior
    }

    PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle& obstacle) override
    {
        return PxControllerBehaviorFlags(0);
    }
};


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
        // // Get the shape we hit
        // PxShape* hitShape = hit.shape;
        //
        // if (hit.worldNormal.y > 0.7f) {  // Adjust threshold as needed
        //     isGrounded = true;
        //    // printf("Ground hit detected!\n");
        // }
        //
        // // Check if the shape has userData
        // if (hitShape->userData != nullptr) {
        //     // Cast userData back to your custom struct
        //     SurfaceData* surfaceData = static_cast<SurfaceData*>(hitShape->userData);
        //
        //     if (surfaceData->climbable) {
        //         printf("Touching a climbable surface!\n");
        //         // You are touching a climbable surface, do whatever logic you need here
        //     }
        // }
    }

    void onControllerHit(const PxControllersHit& hit) override {}
    void onObstacleHit(const PxControllerObstacleHit& hit) override {}
};

void _initializePhysX();
PxRigidStatic* _createStaticMeshCollider(const PxVec3& position, const std::vector<PxVec3>& vertices, const std::vector<PxU32>& indices, bool climbable);
PxController* createPlayerController(const PxVec3& position, float radius, float height);


PxRigidStatic* createStaticMeshCollider(const PxVec3& position, const std::vector<PxVec3>& vertices, const std::vector<PxU32>& indices);


PxRigidStatic* editStaticMeshCollider(PxRigidStatic* existing, const PxVec3& position, const std::vector<PxVec3>& vertices, const std::vector<PxU32>& indices);

void initializePhysX();

void destroyPhysXStuff();


inline static bool FLY_MODE = false;

#endif //PHYSXSTUFF_H
