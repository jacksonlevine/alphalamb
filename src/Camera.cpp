//
// Created by jack on 12/22/2024.
//
#include "Camera.h"

namespace jl
{

    glm::mat4 Camera::model = glm::mat4(1.0f);

    void Camera::updateProjection(int screenwidth, int screenheight, float fov, float farr)
    {
        if(screenheight != 0)
        {
            projection = glm::perspective(
            glm::radians(fov),
            static_cast<float>(screenwidth) / static_cast<float>(screenheight),
            0.1f,
            farr
            );
            mvp = projection * view * model;
        }

    }

    void Camera::setYawPitch(float nyaw, float npitch) {
        transform.yaw = nyaw;
        transform.pitch = npitch;

     }

    //Move actual yaw pitch toward target yaw pitch
    void Camera::interpTowardTargetYP(float deltatime)
    {


        float t = std::min(1.0f, deltatime * 30.0f);

        float deltaYaw = targetYaw - unwrappedYaw;
        unwrappedYaw = unwrappedYaw + deltaYaw * t;


        transform.yaw = unwrappedYaw;
        // Normalize yaw to [0, 360] to prevent it from going out of bounds
        if (transform.yaw > 360.0f) transform.yaw -= 360.0f;
        if (transform.yaw < 0.0f) transform.yaw += 360.0f;

        transform.pitch = glm::mix(transform.pitch, targetPitch, t);

        // Debug prints
        // std::cout << "Target Yaw: " << targetYaw << std::endl;
        // std::cout << "Current Yaw: " << transform.yaw << std::endl;
    }

    //Update the target yaw pitch
    void Camera::updateYPIndirect(float newTargetYaw, float newTargetPitch)
    {
        targetYaw = newTargetYaw;

        // Clamp pitch to prevent gimbal lock
        targetPitch = glm::clamp(newTargetPitch, -89.0f, 89.0f);
    }


    //Update the matrices with whatever yawpitch currently are
    void Camera::updateWithYawPitch(float nyaw, float npitch)
    {
      //std::cout << "Cam being update wit yaw: " << nyaw << " Pitch: " << npitch << std::endl;
        transform.updateWithYawPitch(nyaw, npitch);
        if (glm::dot<3, float, glm::qualifier::highp>(transform.direction, transform.up) == 0.f) {
            if (std::abs(transform.direction.x) > std::abs(transform.direction.z)) {
                if (transform.direction.x > 0) {
                    transform.up = glm::vec3(1, 0, 0);
                }
                else {
                    transform.up = glm::vec3(-1, 0, 0);
                }
            }
            else {
                if (transform.direction.z > 0) {
                    transform.up = glm::vec3(0, 0, 1);
                }
                else {
                    transform.up = glm::vec3(0, 0, -1);
                }
            }
        }
        else {
            transform.up = glm::vec3(0.f, 1.f, 0.f);
            view = glm::lookAt(transform.position,
                transform.position + transform.direction,
                transform.up);

            mvp = projection * view * model;
        }

     
    }
}