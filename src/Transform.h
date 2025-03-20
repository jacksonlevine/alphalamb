//
// Created by jack on 12/22/2024.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "PrecompHeader.h"

namespace jl
{
    class Transform {
    public:
        float yaw = 0.0f;
        float pitch = 0.0f;
        glm::vec3 direction = {};
        glm::vec3 position = {};
        glm::vec3 right = {};
        glm::vec3 up = {};
        glm::vec3 velocity = {};
        bool grounded = false;
        Transform() :
            yaw(0.0),
            pitch(0.0),
            direction(glm::vec3(0.0, 0.0, 1.0)),
            position(glm::vec3(0.0, 200.0, 0.0)),
            right(glm::vec3(1.0, 0.0, 0.0)),
            up(glm::vec3(0.0, 1.0, 0.0)),
            velocity(glm::vec3(0.0, 0.0, 0.0)),
        grounded(false)
        {}
        Transform(float yaw, float pitch) :
            Transform()
        {
            this->updateWithYawPitch(yaw, pitch);
        }
        Transform(glm::vec3 position, float yaw, float pitch) :
            Transform(yaw, pitch)
        {
            this->position = position;
            this->updateWithYawPitch(yaw, pitch);
        }
        void updateWithYawPitch(float nyaw, float npitch);

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(position, direction, yaw, pitch);
        }
    };
}

#endif //TRANSFORM_H
