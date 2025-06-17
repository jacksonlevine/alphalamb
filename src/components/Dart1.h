//
// Created by jack on 6/16/2025.
//

#ifndef DART1_H
#define DART1_H

#include "../PrecompHeader.h"

struct Dart1 {
    glm::vec3 startingDirection;
    float damage;
};

struct Scene;
void renderDart1s(entt::registry& reg, Scene* scene, float deltaTime);

#endif //DART1_H
