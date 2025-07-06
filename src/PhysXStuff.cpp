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






void _initializePhysX() {
    // Step 1: Create the foundation
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    // Step 2: Create PVD (Optional for debugging, visualization)
    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 30000);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    // Step 3: Create physics instance
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), false, gPvd);

    // Step 4: Create the dispatcher for multithreaded simulation
    gDispatcher = PxDefaultCpuDispatcherCreate(2); // 2 threads for physics calculations

    // Step 5: Create the scene
    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = CustomFilterShader;

    sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION;

    gScene = gPhysics->createScene(sceneDesc);

#ifndef NDEBUG
    gScene->getScenePvdClient()->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
    gScene->getScenePvdClient()->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
#endif

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
    static auto mbc = MyBehaviorCallback();

    PxFilterData controllerFilterData;
    controllerFilterData.word0 = static_cast<unsigned int>(CollisionGroup::GROUP_PLAYER);
    controllerFilterData.word1 = static_cast<unsigned int>(CollisionGroup::GROUP_WORLD);


    mbc.playerFilterData = controllerFilterData;

    PxBoxControllerDesc desc;
    desc.halfHeight = height;
    desc.halfSideExtent = radius;

    desc.position = PxExtendedVec3(position.x, position.y, position.z); // Start position
    desc.slopeLimit = 0.707f; // Limit slope climbing
    desc.contactOffset = 0.1f; // Distance to detect collisions
    desc.stepOffset = 0.5f; // Max step height the player can walk up
    desc.upDirection = PxVec3(0.0f, 1.0f, 0.0f); // Gravity direction (Y-up)
    desc.material = gPhysics->createMaterial(0.2f, 0.2f, 0.1f); // Friction material
    desc.nonWalkableMode = PxControllerNonWalkableMode::ePREVENT_CLIMBING; // Prevent climbing steep slopes
    desc.behaviorCallback = &mbc;
    //auto* myHitReport = new MyControllerHitReport();
    //desc.reportCallback = myHitReport;



    // 2. Create the controller using the controller manager
    PxController* playerController = gControllerManager->createController(desc);

    //playerController->setUserData(myHitReport);
    if (!playerController) {
        std::cerr<< "Failed to create controller" << std::endl;
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

            setCollisionFilter(shapes[i], static_cast<uint32_t>(CollisionGroup::GROUP_PLAYER), static_cast<int>(CollisionGroup::GROUP_WORLD) | static_cast<int>(CollisionGroup::GROUP_ANIMAL));
        }
    }

    return playerController;
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
       // std::cout << "Invalid geometry: insufficient vertices or indices" << std::endl;
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

        static PxMaterial* material =  gPhysics->createMaterial(0.2f, 0.2f, 0.1f);

        // Cook the mesh
        PxTolerancesScale scale;
        PxCookingParams cookingParams(scale);
        cookingParams.meshPreprocessParams |= PxMeshPreprocessingFlag::eWELD_VERTICES;
        cookingParams.meshWeldTolerance = 0.1f;
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
                        setCollisionFilter(shape, static_cast<uint32_t>(CollisionGroup::GROUP_WORLD), 0xFFFFFFFF);
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
    //std::cout << "Returning a default rigid static actor without shapes" << std::endl;
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
    cookingParams.meshPreprocessParams |= PxMeshPreprocessingFlag::eWELD_VERTICES;
    cookingParams.meshWeldTolerance = 0.1f;
    PxDefaultMemoryOutputStream writeBuffer;
    PxTriangleMeshCookingResult::Enum result;
    bool status = PxCookTriangleMesh(cookingParams, meshDesc, writeBuffer, &result);

    if (!status || result == PxTriangleMeshCookingResult::eFAILURE ||
        result == PxTriangleMeshCookingResult::eEMPTY_MESH)
    {

        return existing;
    }

    switch (result)
    {
    case PxTriangleMeshCookingResult::eLARGE_TRIANGLE:

        break;
    case PxTriangleMeshCookingResult::eSUCCESS:
        break;
    }

    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    PxTriangleMesh* triangleMesh = gPhysics->createTriangleMesh(readBuffer);

    if (!triangleMesh)
    {

        return existing;
    }

    // Create new shape with a shared material
    PxTriangleMeshGeometry meshGeometry(triangleMesh, PxMeshScale(1.0f));
    PxMaterial* material = gPhysics->createMaterial(0.2f, 0.2f, 0.1f);
    PxShape* newShape = gPhysics->createShape(meshGeometry, *material);

    // Release the material as it's now referenced by the shape
    material->release();

    if (!newShape)
    {
        triangleMesh->release();  // Clean up the mesh if shape creation failed
        return existing;
    }

    setCollisionFilter(newShape, static_cast<uint32_t>(CollisionGroup::GROUP_WORLD), 0xFFFFFFFF);


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

// bool isControllerValid(PxControllerManager* manager, const PxController* controller) {
//     if (!manager || !controller) return false;
//
//     PxU32 numControllers = manager->getNbControllers();
//     for (PxU32 i = 0; i < numControllers; i++) {
//         if (manager->getController(i) == controller) {
//             return true;
//         }
//     }
//     return false;
// }