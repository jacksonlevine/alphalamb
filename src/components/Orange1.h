//
// Created by jack on 6/12/2025.
//

#ifndef ORANGE1_H
#define ORANGE1_H
#include "../IntTup.h"


struct Orange1
{
    int8_t health = 100;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(health);
    }
};
struct Scene;
std::array<glm::vec3, 7> getJungleCampOffsetsForWorldXZ(FastNoiseLite& noise, const TwoIntTup& wxz);
void renderOrange1Guys(entt::registry& reg, Scene* scene, float deltaTime);

#endif //ORANGE1_H
