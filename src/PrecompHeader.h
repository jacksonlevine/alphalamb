//
// Created by jack on 1/5/2025.
//

#ifndef PRECOMPHEADER_H
#define PRECOMPHEADER_H

#include <stdio.h>
#include "GLHeaders.h"
#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <boost/asio.hpp>
#include <boost/unordered_set.hpp>
#include <boost/container/vector.hpp>
#include <glm\glm.hpp>
#include <physx/PxPhysicsAPI.h>
#include "boost/lockfree/spsc_queue.hpp"
#include <physx/cooking/PxCooking.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_gltf.h>
#include <stb_image.h>
#include <PxRigidStatic.h>
#include <cassert>
#include <unordered_set>
#include <memory>
#include <filesystem>
#include <ranges>
#include <iterator>
#include <algorithm>
#include "FastNoiseLite.h"
#include <boost/unordered_map.hpp>
#include <variant>
#include <AL/al.h>
#include <AL/alc.h>
#include <sndfile.h>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <bitset>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <boost/uuid/string_generator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <entt/entt.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include "CerealHelpers.h"
#include <boost/thread/concurrent_queues/sync_queue.hpp>
#include <tbb/tbb.h>
#include <tbb/concurrent_unordered_set.h>
#include <tuple>
#include <boost/thread/thread.hpp>
#include <boost/thread/thread_pool.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

constexpr double J_PI = 3.1415926535897932384626433832;
extern GLuint lutTexture;
#define DAY_LENGTH 900.0f

#endif //PRECOMPHEADER_H
