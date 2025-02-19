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
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 mvp;
        float targetYaw = 0.0f;
        float targetPitch = 0.0f;

        float unwrappedYaw = 0.0f;
        float unwrappedPitch = 0.0f;



        void interpTowardTargetYP(float deltatime);
        Camera()
            : //model(glm::mat4(1.0f)),
              view(glm::mat4(1.0f)),
              projection(glm::mat4(1.0f)),
              mvp(glm::mat4(1.0f))
        {}
        void updateProjection(int screenwidth, int screenheight, float fov);
        void updateWithYawPitch(float nyaw, float npitch);
        void updateYPIndirect(float nyaw, float npitch);
        void setYawPitch(float nyaw, float npitch);
    };
}

#endif //CAMERA_H
