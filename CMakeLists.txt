cmake_minimum_required(VERSION 3.28)

# Suppress CMP0167 warning due to vcpkg's use of legacy FindBoost
#cmake_policy(SET CMP0167 OLD)

# Set the vcpkg toolchain file, make sure you do this before project()
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
else()
    message(FATAL_ERROR "VCPKG_ROOT environment variable not set")
endif()

project(DistantGarden)

add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /Ob2 /DNDEBUG") # Ensure NDEBUG is set in Release mode
set(CMAKE_CXX_FLAGS_DEBUG "/Zi /D_DEBUG")    # Ensure _DEBUG is set in Debug mode


find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
find_package(EnTT CONFIG REQUIRED)

find_package(Boost REQUIRED COMPONENTS asio uuid lockfree container variant2 fiber)
message(STATUS "Boost version: ${Boost_VERSION}")
find_package(imgui CONFIG REQUIRED)

find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)



# Print all available targets
message(STATUS "Available targets in CMake:")
get_property(TARGETS_LIST GLOBAL PROPERTY TARGETS)
foreach(TARGET ${TARGETS_LIST})
    message(STATUS "Target: ${TARGET}")
endforeach()

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
        src/ImGuiStuff.cpp
        src/ImGuiStuff.h
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
        src/AtomicRWInt.h
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
        src/world/gizmos/DropsGizmo.cpp
        src/world/gizmos/DropsGizmo.h
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
        src/creatures/CreatureWorld.cpp
        src/creatures/CreatureWorld.h
)

target_link_libraries(DistantGarden PRIVATE OpenAL::OpenAL)
function(add_common_libraries target)
    
    target_precompile_headers( ${target}
            PRIVATE ${CMAKE_SOURCE_DIR}/src/PrecompHeader.h )

    target_link_libraries(${target} PRIVATE EnTT::EnTT)
    
    # Include directories
    target_include_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    
    target_link_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/lib)
    target_include_directories(${target} PRIVATE ${TINYGLTF_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE imgui::imgui)
    target_link_libraries(${target} PRIVATE Boost::asio Boost::uuid Boost::fiber)
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



