//
// Created by jack on 3/23/2025.
//

#ifndef DRAWSKY_H
#define DRAWSKY_H
#include "Camera.h"


class World;
void dgDrawSky(const glm::vec3& pos, GLuint lutTexture, World& world, float timeOfDay, jl::Camera* cam);



#endif //DRAWSKY_H
