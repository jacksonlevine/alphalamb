//
// Created by jack on 2/20/2025.
//

#ifndef SUNANDMOON_H
#define SUNANDMOON_H

#include "AtomicRWInt.h"
#include "AtomicRWInt.h"
#include "PrecompHeader.h"
#include "Shader.h"
#include "Camera.h"

struct Scene;
void drawSunAndMoon(jl::Camera* camera, float timeOfDay, float dayLength, const glm::vec3& postr);

#endif //SUNANDMOON_H
