//
// Created by jack on 12/22/2024.
//
#include "Transform.h"

namespace jl
{

    void Transform::updateWithYawPitch(const float nyaw, const float npitch)
    {

        pitch = npitch;
        yaw = nyaw;
        if(pitch > 89.999f)
        {
            pitch = 89.999f;
        }
        if(pitch < -89.999f)
        {
            pitch = -89.999f;
        }
        yaw = fmod(yaw, 360.0f);
        if (yaw < 0.0f)
        {
            yaw += 360.0f;
        }
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction = glm::normalize(direction);
        right = -1.0f * glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));
        up = -1.0f * glm::normalize(glm::cross(direction, right));

    }

}