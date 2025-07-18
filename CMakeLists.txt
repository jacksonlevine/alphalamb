cmake_minimum_required(VERSION 3.28)

# Suppress CMP0167 warning due to vcpkg's use of legacy FindBoost
#cmake_policy(SET CMP0167 OLD)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
else()
    message(FATAL_ERROR "VCPKG_ROOT environment variable not set")
endif()

if (MSVC)
    add_compile_options(/fsanitize=address)
    add_link_options(/INFERASANLIBS)
    add_link_options(/INCREMENTAL:NO)
endif()

project(DistantGarden)

add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)

set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /Ob2 /DNDEBUG /m /MP") # Ensure NDEBUG is set in Release mode
set(CMAKE_CXX_FLAGS_DEBUG "/Zi /D_DEBUG")    # Ensure _DEBUG is set in Debug mode


find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
find_package(EnTT CONFIG REQUIRED)
# this is heuristically generated, and may not be correct
find_package(rapidcheck CONFIG REQUIRED)


find_package(Boost REQUIRED COMPONENTS system asio uuid lockfree container variant2 thread exception)
message(STATUS "Boost version: ${Boost_VERSION}")
find_package(imgui CONFIG REQUIRED)

find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)


message(STATUS "Available targets in CMake:")
get_property(TARGETS_LIST GLOBAL PROPERTY TARGETS)
foreach(TARGET ${TARGETS_LIST})
    message(STATUS "Target: ${TARGET}")
endforeach()


add_library(pocketpy STATIC
        thirdparty/pocketpy/pocketpy.c
)
target_include_directories(pocketpy PUBLIC
        thirdparty/pocketpy
)
set_target_properties(pocketpy PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
)



add_executable(DistantGarden
        src/main.cpp
        src/BasicShader.h
        src/Camera.cpp
        src/Camera.h
        src/Controls.cpp
        src/Controls.h
        src/FastNoiseLite.h
        src/IntTup.cpp
        src/IntTup.h
        src/Shader.cpp
        src/Shader.h
        src/Texture.cpp
        src/Texture.h
        src/Player.h
        src/Player.cpp
        src/PhysXStuff.h
        src/PhysXStuff.cpp
        src/Transform.h
        src/Transform.cpp
        src/world/DataMap.h
        src/world/WorldGenMethod.h
        src/world/worldgenmethods/OverworldWorldGenMethod.cpp
        src/world/worldgenmethods/OverworldWorldGenMethod.h
        src/world/datamapmethods/HashMapDataMap.cpp
        src/world/datamapmethods/HashMapDataMap.h
        src/world/MaterialName.h
        src/world/World.cpp
        src/world/World.h
        src/world/WorldRenderer.cpp
        src/world/WorldRenderer.h
        src/CollisionCage.cpp
        src/CollisionCage.h
        src/world/WorldGizmo.h
        src/world/gizmos/BlockSelectGizmo.cpp
        src/world/gizmos/BlockSelectGizmo.h
        src/Sky.h
        src/world/VoxelModels.cpp
        src/world/VoxelModels.h
        src/world/MaterialName.cpp
        src/world/gizmos/ParticlesGizmo.cpp
        src/world/gizmos/ParticlesGizmo.h
        src/world/SWCLoader.cpp
        src/world/SWCLoader.h
        src/LUTLoader.h
        src/ImGuiStuff.h
        src/ImGuiStuff.cpp
        src/world/RebuildQueue.h
        src/Image.cpp
        src/Image.h
        src/Hud.cpp
        src/Hud.h
        src/Hud.cpp
        src/NetworkTypes.cpp
        src/NetworkTypes.h
        src/SharedVarsBetweenMainAndGui.h
        src/Server.cpp
        src/Server.h
        src/Client.cpp
        src/Client.h
        src/Scene.h
        src/BillboardInstanceShader.cpp
        src/BillboardInstanceShader.h
        src/Texture2DArray.cpp
        src/Texture2DArray.h
        src/HandledBlock.h
        src/world/gizmos/BulkPlaceGizmo.cpp
        src/world/gizmos/BulkPlaceGizmo.h
        src/IndexOptimization.h
        src/OpenALStuff.cpp
        src/OpenALStuff.h
        src/world/gizmos/VoxModelStampGizmo.cpp
        src/world/gizmos/VoxModelStampGizmo.h
        src/LocalServerIOContext.h
        src/LocalServerIOContext.cpp
        src/FPSCounter.cpp
        src/FPSCounter.h
        src/ModelLoader.cpp
        src/ModelLoader.h
        src/TextureFace.cpp
        src/TextureFace.h
        src/Inventory.cpp
        src/Inventory.h
        src/SunAndMoon.h
        src/TimeOfDay.h
        src/Planets.cpp
        src/Planets.h
        src/world/ItemName.h
        src/EnumToString.h
        src/BlockType.h
        src/ClientUID.h
        src/PlayerInfoMapKeyedByUID.h
        src/ClientUID.cpp
        src/menupage/FullscreenKaleidoscope.h
        src/AmbOcclSetting.cpp
        src/AmbOcclSetting.h
        src/specialblocks/SpecialBlockInfo.h
        src/specialblocks/Fence.h
        src/specialblocks/FindSpecialBlock.h
        src/specialblocks/Stairs.h
        src/components/InventoryComponent.h
        src/components/MovementComponent.h
        src/components/ParticleEffectComponent.h
        src/components/NetworkComponent.h
        src/components/RenderComponent.h
        src/components/PhysicsComponent.h
        src/components/PhysicsComponent.cpp
        src/components/PlayerEmplacer.h
        src/FileArchives.h
        src/CerealHelpers.h
        src/SaveRegistry.h
        src/components/UUIDComponent.h
        src/SaveRegistry.cpp
        src/world/FootstepSounds.cpp
        src/world/FootstepSounds.h
        src/DrawSky.cpp
        src/DrawSky.h
        src/LUTLoader.cpp
        src/SunAndMoon.cpp
        src/BasicShader.cpp
        src/BasicGLTFShader.cpp
        src/PythonContext.cpp
        src/PythonContext.h
        src/TextEditor.cpp
        src/TextEditor.h
        src/specialblocks/Computer.h
        src/components/ComputerComponent.cpp
        src/components/ComputerComponent.h
        src/components/NPPositionComponent.cpp
        src/components/NPPositionComponent.h
        src/specialblocks/FindEntityCreateFunc.h
        src/specialblocks/Door.h
        src/specialblocks/Cable.h
        src/Light.cpp
        src/Light.h
        src/LightSpeedTest.cpp
        src/LightSpeedTest.h
        src/StepTimerProfiler.h
        src/Helpers.h
        src/world/datamapmethods/HashMapDataMapTemplate.h
        src/jl484_vec3.h
        src/components/WorldStateComponent.h
        src/components/LootDrop.h
        src/components/LootDrop.cpp
        src/components/Factories.cpp
        src/components/Factories.h
        src/EntityWithNameAdder.h
        src/FlatCposSet.h
        src/components/StormyCloud.h
        src/components/Lifetime.h
        src/components/PlayerComp.h
        src/Recipes.h
        src/components/Orange1.cpp
        src/components/Orange1.h
        src/components/JungleCampSpawnedInChunk.h
        src/components/HealthComponent.h
        src/components/Dart1.cpp
        src/components/Dart1.h
        src/components/SoundSource.h
        src/InstancedThingyTemplate.h
        src/MarchingCubesLookups.h
)



add_executable(DGServer
        src/servermain.cpp
        src/Server.h
        src/Server.cpp
        src/PrecompHeader.h
        src/IntTup.cpp
        src/world/datamapmethods/HashMapDataMap.cpp
        src/world/DataMap.h
        src/Camera.cpp
        src/Camera.h
        src/Transform.cpp
        src/Transform.h
        src/ClientUID.h
        src/ClientUID.cpp
        src/components/InventoryComponent.h
        src/components/MovementComponent.h
        src/components/ParticleEffectComponent.h
        src/components/NetworkComponent.h
        src/components/RenderComponent.h
        src/components/PhysicsComponent.h
        src/components/PhysicsComponent.cpp
        src/components/PlayerEmplacer.h
        src/FileArchives.h
        src/CerealHelpers.h
        src/SaveRegistry.h
        src/components/UUIDComponent.h
        src/SaveRegistry.cpp
        src/PythonContext.cpp
        src/PythonContext.h
        src/components/ComputerComponent.cpp
        src/components/ComputerComponent.h
        src/components/NPPositionComponent.cpp
        src/components/NPPositionComponent.h
        src/specialblocks/FindEntityCreateFunc.h
        src/specialblocks/Door.h
        src/specialblocks/Cable.h
        src/Light.cpp
        src/Light.h

)

target_link_libraries(DistantGarden PRIVATE OpenAL::OpenAL)


function(add_common_libraries target)

    # note: 3 additional targets are not displayed.
    target_link_libraries(${target} PRIVATE rapidcheck rapidcheck_boost)
    
    target_precompile_headers(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src/PrecompHeader.h)

    target_link_libraries(${target} PRIVATE EnTT::EnTT)
    target_link_libraries(${target} PRIVATE pocketpy)
    target_link_libraries(${target} PRIVATE TBB::tbb)

    target_link_libraries(${target} PRIVATE cereal::cereal)
    # Include directories
    target_include_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    
    target_link_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/lib)
    target_include_directories(${target} PRIVATE ${TINYGLTF_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE imgui::imgui)
    target_link_libraries(${target} PRIVATE Boost::asio Boost::uuid Boost::thread Boost::system Boost::exception)
    target_link_libraries(${target} PRIVATE Boost::lockfree)
    
    
    # target_link_libraries(${target} PRIVATE SharedThings)
    
    target_link_libraries(${target} PRIVATE     unofficial::omniverse-physx-sdk::PhysXCooking
            unofficial::omniverse-physx-sdk::PhysXFoundation
            unofficial::omniverse-physx-sdk::PhysXCommon
            unofficial::omniverse-physx-sdk::sdk)
    
    # Optional: import the defined target to copy over the GPU acceleration libraries (3rd party provided by NVIDIA)
    if(TARGET unofficial::omniverse-physx-sdk::gpu-library)
        if(UNIX)
            # Add rpath setting to find .so libraries on unix based systems
            set_target_properties(${target} PROPERTIES
                    BUILD_WITH_INSTALL_RPATH TRUE
                    INSTALL_RPATH "$ORIGIN"
            )
        endif()
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:unofficial::omniverse-physx-sdk::gpu-library>
                $<TARGET_FILE_DIR:${target}>)
        if(WIN32)
            add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:unofficial::omniverse-physx-sdk::gpu-device-library>
                    $<TARGET_FILE_DIR:${target}>)
        endif()
    else()
        message(WARNING "GPU acceleration library target not defined - GPU acceleration will NOT be available!")
    endif()
    target_link_libraries(${target} PRIVATE glfw glm::glm GLEW::GLEW)
    target_link_libraries(${target} PRIVATE SndFile::sndfile)
endfunction()


add_common_libraries(DistantGarden)
add_common_libraries(DGServer)



