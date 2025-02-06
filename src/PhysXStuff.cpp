//
// Created by jack on 1/6/2025.
//

#include "PhysXStuff.h"

using namespace physx;

PxDefaultAllocator gAllocator;
PxDefaultErrorCallback gErrorCallback;

PxFoundation* gFoundation = nullptr;
PxPhysics* gPhysics = nullptr;
PxDefaultCpuDispatcher* gDispatcher = nullptr;
PxScene* gScene = nullptr;
PxPvd* gPvd = nullptr;
PxControllerManager* gControllerManager = nullptr;

SurfaceData::SurfaceData()
{
    climbable = false;
}


// PxFilterFlags CustomFilterShader(
//     PxFilterObjectAttributes attributes0, PxFilterData filterData0,
//     PxFilterObjectAttributes attributes1, PxFilterData filterData1,
//     PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize) {
//
//     printf("Collision Check: filterData0.word0 = %d, filterData1.word0 = %d\n",
//             filterData0.word0, filterData1.word0);
//
//     if (filterData0.word0 == 2 || filterData1.word0 == 2) {
//         if (filterData0.word0 == 1 || filterData1.word0 == 1) {
//             printf("Allowing collision: Controller <-> Static Body\n");
//             pairFlags = PxPairFlag::eCONTACT_DEFAULT;
//             return PxFilterFlag::eDEFAULT;
//         } else {
//             printf("Suppressing collision: Controller <-> Non-static body\n");
//             pairFlags = PxPairFlag::eTRIGGER_DEFAULT; // Make sure it's only a trigger, not a physical collision
//             return PxFilterFlag::eSUPPRESS;
//         }
//     }
//
//
//     // Default behavior for other collisions
//     pairFlags = PxPairFlag::eCONTACT_DEFAULT;
//     return PxFilterFlag::eDEFAULT;
// }
//

void _initializePhysX() {
    // Step 1: Create the foundation
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    // Step 2: Create PVD (Optional for debugging, visualization)
    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    // Step 3: Create physics instance
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

    // Step 4: Create the dispatcher for multithreaded simulation
    gDispatcher = PxDefaultCpuDispatcherCreate(2); // 2 threads for physics calculations

    // Step 5: Create the scene
    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);

    gControllerManager = PxCreateControllerManager(*gScene);

    // Add PVD client to the scene
    PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
    if (pvdClient) {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }


}


PxController* createPlayerController(const PxVec3& position, float radius, float height) {
    // 1. Configure the capsule controller descriptor
    PxCapsuleControllerDesc desc;
    desc.height = height; // The height of the capsule
    desc.radius = radius; // The radius of the capsule
    desc.position = PxExtendedVec3(position.x, position.y, position.z); // Start position
    desc.slopeLimit = 0.707f; // Limit slope climbing
    desc.contactOffset = 0.01f; // Distance to detect collisions
    desc.stepOffset = 0.5f; // Max step height the player can walk up
    desc.upDirection = PxVec3(0.0f, 1.0f, 0.0f); // Gravity direction (Y-up)
    desc.material = gPhysics->createMaterial(0.5f, 0.5f, 0.1f); // Friction material
    desc.nonWalkableMode = PxControllerNonWalkableMode::ePREVENT_CLIMBING; // Prevent climbing steep slopes

    MyControllerHitReport* myHitReport = new MyControllerHitReport();
    desc.reportCallback = myHitReport;



    PxFilterData controllerFilterData;
    controllerFilterData.word0 = 2;
    controllerFilterData.word1 = 1; //Collides with 1

    // 2. Create the controller using the controller manager
    PxController* playerController = gControllerManager->createController(desc);

    playerController->setUserData(myHitReport);
    if (!playerController) {
        throw std::runtime_error("Failed to create player controller");
    }

    // 3. Retrieve the actor and set its name
    PxRigidDynamic* playerActor = playerController->getActor();
    if (playerActor) {
        playerActor->setName("Player");
        // 4. Retrieve the shape(s) and set the collision filter
        PxShape* shapes[16]; // Buffer to store shapes (assuming 16 or fewer)
        PxU32 shapeCount = playerActor->getShapes(shapes, 16); // Get all shapes

        // 5. Set the filter data for each shape
        for (PxU32 i = 0; i < shapeCount; i++) {

            shapes[i]->setSimulationFilterData(controllerFilterData);
        }
    }

    // // 4. Set the collision filter for the actor's shape(s)
    // physx::PxRigidDynamic* playerActor = playerController->getActor(); // Get underlying actor
    // physx::PxShape* shapes[16]; // Buffer to store shapes (assuming 16 or fewer)
    // physx::PxU32 shapeCount = playerActor->getShapes(shapes, 16); // Get all shapes
    //
    // // 5. Set the filter for each shape of the player
    // for (physx::PxU32 i = 0; i < shapeCount; i++) {
    //     PxFilterData filterData;
    //     filterData.word0 = 1 << 0; // Assign to a specific group (e.g., "particles")
    //     filterData.word1 = 1 << 0; // Should not collide with itself
    //     shapes[i]->setSimulationFilterData(filterData);
    // }

    return playerController;
}
PxRigidDynamic* createPlayerKinematic(const PxVec3& position, float radius, float halfHeight) {
    // Create capsule geometry
    PxCapsuleGeometry capsuleGeometry(radius, halfHeight);

    // Create kinematic dynamic actor (note: use PxRigidDynamic, not PxRigidStatic)
    PxRigidDynamic* player = gPhysics->createRigidDynamic(PxTransform(position));

    // 1. Create the shape using the PxPhysics object
    PxShape* shape = gPhysics->createShape(capsuleGeometry, *gPhysics->createMaterial(0.5f, 0.5f, 0.1f));

    // 2. Attach the shape to the player (PxRigidDynamic)
    player->attachShape(*shape);

    // Set the kinematic flag
    player->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

    // Add to the PhysX scene
    gScene->addActor(*player);

    return player;
}
PxRigidStatic* _createStaticMeshCollider(const PxVec3& position, const std::vector<PxVec3>& vertices, const std::vector<PxU32>& indices, bool climbable);
PxRigidStatic* createStaticMeshCollider(const PxVec3& position, const std::vector<PxVec3>& vertices, const std::vector<PxU32>& indices)
{
    return _createStaticMeshCollider(position, vertices, indices, false);
}

PxRigidStatic* _createStaticMeshCollider(const PxVec3& position,
    const std::vector<PxVec3>& vertices, const std::vector<PxU32>& indices, bool climbable)
{
    // Early log for invalid geometry
    if (vertices.empty() || indices.empty() || indices.size() < 3)
    {
        std::cout << "Invalid geometry: insufficient vertices or indices" << std::endl;
    }
    else
    {
        // Create mesh description
        PxTriangleMeshDesc meshDesc;
        meshDesc.points.count = vertices.size();
        meshDesc.points.stride = sizeof(PxVec3);
        meshDesc.points.data = vertices.data();

        meshDesc.triangles.count = indices.size() / 3;
        meshDesc.triangles.stride = 3 * sizeof(PxU32);
        meshDesc.triangles.data = indices.data();

        static PxMaterial* material =  gPhysics->createMaterial(0.5f, 0.5f, 0.1f);

        // Cook the mesh
        PxTolerancesScale scale;
        PxCookingParams cookingParams(scale);
        cookingParams.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
        //cookingParams.meshWeldTolerance = 0.0f;
        PxDefaultMemoryOutputStream writeBuffer;
        PxTriangleMeshCookingResult::Enum result;
        bool status = PxCookTriangleMesh(cookingParams, meshDesc, writeBuffer, &result);

        if (status && result != PxTriangleMeshCookingResult::eFAILURE &&
            result != PxTriangleMeshCookingResult::eEMPTY_MESH)
        {
            // Create triangle mesh
            PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
            PxTriangleMesh* triangleMesh = gPhysics->createTriangleMesh(readBuffer);

            if (triangleMesh)
            {
                // Create rigid static actor
                PxRigidStatic* staticActor = gPhysics->createRigidStatic(PxTransform(position));
                if (staticActor)
                {
                    PxTriangleMeshGeometry meshGeometry(triangleMesh, PxMeshScale(1.0f));
                    PxShape* shape = gPhysics->createShape(meshGeometry, *material);
                    if (shape)
                    {

                        // Set the collision filtering
                        PxFilterData filterData;
                        filterData.word0 = 1;  // Set to group 1
                        shape->setSimulationFilterData(filterData);
                        shape->setQueryFilterData(filterData);

                        if (climbable)
                        {
                            shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
                        }

                        staticActor->attachShape(*shape);
                        shape->release();
                        gScene->addActor(*staticActor);
                        return staticActor;
                    }
                    triangleMesh->release();
                }
                triangleMesh->release();
            }
        }
    }

    // Create and return a basic rigid static actor if the process fails
    std::cout << "Returning a default rigid static actor without shapes" << std::endl;
    PxRigidStatic* defaultActor = gPhysics->createRigidStatic(PxTransform(position));
    if (defaultActor)
    {
        gScene->addActor(*defaultActor);
    }
    return defaultActor;
}


PxRigidStatic* editStaticMeshCollider(PxRigidStatic* existing, const PxVec3& position,
    const std::vector<PxVec3>& vertices, const std::vector<PxU32>& indices)
{
    // Early returns for empty geometry
    if (vertices.empty() || indices.empty() || indices.size() < 3)
    {
        // If there's an existing shape, we should remove it since we have no geometry
        PxShape* currentShape = nullptr;
        if (existing->getNbShapes() > 0)
        {
            existing->getShapes(&currentShape, 1);
            if (currentShape)
            {
                // detachShape handles the shape release
                existing->detachShape(*currentShape);
            }
        }
        return existing;
    }

    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = vertices.size();
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = vertices.data();

    meshDesc.triangles.count = indices.size() / 3;
    meshDesc.triangles.stride = 3 * sizeof(PxU32);
    meshDesc.triangles.data = indices.data();

    PxTolerancesScale scale;
    PxCookingParams cookingParams(scale);
    cookingParams.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
    //cookingParams.meshWeldTolerance = 0.0f;
    PxDefaultMemoryOutputStream writeBuffer;
    PxTriangleMeshCookingResult::Enum result;
    bool status = PxCookTriangleMesh(cookingParams, meshDesc, writeBuffer, &result);

    if (!status || result == PxTriangleMeshCookingResult::eFAILURE ||
        result == PxTriangleMeshCookingResult::eEMPTY_MESH)
    {
        std::cout << "Mesh cooking failed or empty, result: " << std::to_string(result) << std::endl;
        return existing;
    }

    switch (result)
    {
    case PxTriangleMeshCookingResult::eLARGE_TRIANGLE:
        std::cout << "Large triangle mesh cooking result" << std::endl;
        break;
    case PxTriangleMeshCookingResult::eSUCCESS:
        break;
    }

    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    PxTriangleMesh* triangleMesh = gPhysics->createTriangleMesh(readBuffer);

    if (!triangleMesh)
    {
        std::cout << "Failed to create triangle mesh" << std::endl;
        return existing;
    }

    // Create new shape with a shared material
    PxTriangleMeshGeometry meshGeometry(triangleMesh, PxMeshScale(1.0f));
    PxMaterial* material = gPhysics->createMaterial(0.5f, 0.5f, 0.1f);
    PxShape* newShape = gPhysics->createShape(meshGeometry, *material);

    // Release the material as it's now referenced by the shape
    material->release();

    if (!newShape)
    {
        std::cout << "Failed to create shape" << std::endl;
        triangleMesh->release();  // Clean up the mesh if shape creation failed
        return existing;
    }

    // Set the collision filtering
    PxFilterData filterData;
    filterData.word0 = 1;  // Set to group 1
    newShape->setSimulationFilterData(filterData);
    newShape->setQueryFilterData(filterData);


    if(existing)
    {
        // Remove existing shape if present
        PxShape* currentShape = nullptr;
        if (existing->getNbShapes() > 0)
        {
            existing->getShapes(&currentShape, 1);
            if (currentShape)
            {
                // Get the old triangle mesh before detaching the shape
                PxGeometryHolder geom = currentShape->getGeometry();
                if (geom.getType() == PxGeometryType::eTRIANGLEMESH)
                {
                    PxTriangleMesh* oldMesh = geom.triangleMesh().triangleMesh;
                    if (oldMesh)
                    {
                        // Increment the ref count since we're holding onto it
                        oldMesh->acquireReference();

                        // Now detach the shape (which will decrease mesh ref count via shape release)
                        existing->detachShape(*currentShape);

                        // Now release our reference to the mesh
                        oldMesh->release();
                    }
                }
                else
                {
                    // If it's not a triangle mesh, just detach the shape
                    existing->detachShape(*currentShape);
                }
            }
        }

        // Attach new shape
        existing->attachShape(*newShape);
    }

    // Release our reference to the shape (RigidStatic maintains its own reference)
    newShape->release();

    // Release the triangle mesh (the shape maintains its own reference)
    triangleMesh->release();

    return existing;
}
void initializePhysX()
{
    _initializePhysX();

    // std::vector<PxVec3> vertices = {
    //     PxVec3(-50.0f, 0.0f, -50.0f),
    //     PxVec3(50.0f, 0.0f, -50.0f),
    //     PxVec3(50.0f, 0.0f, 50.0f),
    //     PxVec3(-50.0f, 0.0f, 50.0f)
    // };
    // PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gPhysics->createMaterial(0.5f, 0.5f, 0.1f));
    // gScene->addActor(*groundPlane);
    // std::vector<PxU32> indices = {
    //     0, 3, 2,
    //     2, 1, 0
    // };
    //playerBody = createPlayerKinematic(PxVec3(0.0, 5.0, 0.0), 2.0f, 2.0f);
    //createStaticMeshCollider(PxVec3(0.0, -5.0, 0.0), vertices, indices);
}

void destroyPhysXStuff()
{
    gPhysics->release();
    gFoundation->release();
}