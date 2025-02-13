//
// Created by jack on 12/22/2024.
//
#include "Camera.h"

namespace jl
{

    glm::mat4 Camera::model = glm::mat4(1.0f);

    void Camera::updateProjection(int screenwidth, int screenheight, float fov)
    {
        if(screenheight != 0)
        {
            projection = glm::perspective(
            glm::radians(fov),
            static_cast<float>(screenwidth) / static_cast<float>(screenheight),
            0.1f,
            512.0f
            );
            mvp = projection * view * model;
        }

    }

    void Camera::setYawPitch(float nyaw, float npitch) {
        transform.yaw = nyaw;
        transform.pitch = npitch;
     }

    void Camera::interpTowardTargetYP(float deltatime)
    {
        // float deltaYaw = targetyaw - transform.yaw;
        // if (deltaYaw > 180.0f) {
        //     deltaYaw -= 360.0f;
        // } else if (deltaYaw < -180.0f) {
        //     deltaYaw += 360.0f;
        // }
        //
        // transform.yaw += deltaYaw * deltatime;
        // transform.pitch = glm::mix(transform.pitch, targetpitch, deltatime);
        //
        // if (transform.yaw < 0.0f) {
        //     transform.yaw += 360.0f;
        // } else if (transform.yaw >= 360.0f) {
        //     transform.yaw -= 360.0f;
        // }
    }

    void Camera::updateWithYawPitch(float nyaw, float npitch)
    {
      //std::cout << "Cam being update wit yaw: " << nyaw << " Pitch: " << npitch << std::endl;
        transform.updateWithYawPitch(nyaw, npitch);
        view = glm::lookAt(transform.position,
            transform.position + transform.direction,
            transform.up);

        mvp = projection * view * model;
    }
}