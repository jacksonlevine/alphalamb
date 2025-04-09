//
// Created by jack on 12/22/2024.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "Transform.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
namespace jl {
    class Camera {
    public:
        jl::Transform transform = {};
        static glm::mat4 model;
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        glm::mat4 mvp = glm::mat4(1.0f);
        float targetYaw = 0.0f;
        float targetPitch = 0.0f;

        float unwrappedYaw = 0.0f;
        float unwrappedPitch = 0.0f;

        void interpTowardTargetYP(float deltatime);
        Camera()

        {
            updateWithYawPitch(0.0, 0.0);
            updateProjection(1280, 1024, 90.0f);
        }

        void updateProjection(int screenwidth, int screenheight, float fov);
        void updateWithYawPitch(float nyaw, float npitch);
        void updateYPIndirect(float nyaw, float npitch);
        void setYawPitch(float nyaw, float npitch);

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(transform);
        }
    };
}

#endif //CAMERA_H
