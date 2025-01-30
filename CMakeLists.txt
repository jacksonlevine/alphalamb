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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(portaudio CONFIG REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")


find_package(Boost REQUIRED COMPONENTS asio uuid lockfree)
message(STATUS "Boost version: ${Boost_VERSION}")
find_package(imgui CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)

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
        src/world/UserDataMap.h
        src/world/WorldGenMethod.h
        src/world/worldgenmethods/PerlinWorldGenMethod.cpp
        src/world/worldgenmethods/PerlinWorldGenMethod.h
        src/world/userdatamapmethods/HashMapUserDataMap.cpp
        src/world/userdatamapmethods/HashMapUserDataMap.h
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

)

target_precompile_headers( DistantGarden
        PRIVATE ${CMAKE_SOURCE_DIR}/src/PrecompHeader.h )


# Include directories
target_include_directories(DistantGarden PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_directories(DistantGarden PUBLIC ${CMAKE_SOURCE_DIR}/lib)
target_include_directories(DistantGarden PRIVATE ${TINYGLTF_INCLUDE_DIRS})
target_link_libraries(DistantGarden PRIVATE imgui::imgui)
target_link_libraries(DistantGarden PRIVATE Boost::asio Boost::uuid)
target_link_libraries(DistantGarden PRIVATE Boost::lockfree)
target_link_libraries(DistantGarden PRIVATE cereal::cereal)

# target_link_libraries(DistantGarden PRIVATE SharedThings)

target_link_libraries(DistantGarden PRIVATE     unofficial::omniverse-physx-sdk::PhysXCooking
        unofficial::omniverse-physx-sdk::PhysXFoundation
        unofficial::omniverse-physx-sdk::PhysXCommon
        unofficial::omniverse-physx-sdk::sdk)

# Optional: import the defined target to copy over the GPU acceleration libraries (3rd party provided by NVIDIA)
if(TARGET unofficial::omniverse-physx-sdk::gpu-library)
    if(UNIX)
        # Add rpath setting to find .so libraries on unix based systems
        set_target_properties(DistantGarden PROPERTIES
                BUILD_WITH_INSTALL_RPATH TRUE
                INSTALL_RPATH "$ORIGIN"
        )
    endif()
    add_custom_command(TARGET DistantGarden POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:unofficial::omniverse-physx-sdk::gpu-library>
            $<TARGET_FILE_DIR:DistantGarden>)
    if(WIN32)
        add_custom_command(TARGET DistantGarden POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:unofficial::omniverse-physx-sdk::gpu-device-library>
                $<TARGET_FILE_DIR:DistantGarden>)
    endif()
else()
    message(WARNING "GPU acceleration library target not defined - GPU acceleration will NOT be available!")
endif()


target_link_libraries(DistantGarden PRIVATE glfw glm::glm GLEW::GLEW)
target_link_libraries(DistantGarden PRIVATE SndFile::sndfile)
target_link_libraries(DistantGarden PRIVATE portaudio)

