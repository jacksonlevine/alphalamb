//
// Created by jack on 2/20/2025.
//

#ifndef PLANETS_H
#define PLANETS_H



enum class PlanetType {
      Home,
      Hostile,
      Bleak,
      Frozen,
      Icy,
      Terriproge
};


struct Atmosphere
{
      glm::vec3 skyTop = {};
      glm::vec3 skyBottom = {};
      glm::vec3 fogColor = {};
};

constexpr Atmosphere skyAndFogColor(PlanetType type)
{
      switch (type)
      {
            case PlanetType::Home:
                  return Atmosphere {
                  .skyTop = glm::vec3(0.3, 0.65, 1.0),
                  .skyBottom = glm::vec3(1.0, 1.0, 1.0),
                  .fogColor = glm::vec3(0.4, 0.75, 1.0) };
            case PlanetType::Hostile:
                  return Atmosphere {
                  .skyTop = glm::vec3(0.1, 0.1, 0.1),
                  .skyBottom = glm::vec3(0.5, 0.0, 0.0),
                  .fogColor = glm::vec3(0.3, 0.0, 1.0) };
            case PlanetType::Bleak:
                  return Atmosphere {
                  .skyTop = glm::vec3(1.0, 1.0, 1.0),
                  .skyBottom = glm::vec3(1.0, 1.0, 1.0),
                  .fogColor = glm::vec3(0.4, 0.7, 1.0) };
            case PlanetType::Frozen:
                  return Atmosphere {
                  .skyTop = glm::vec3(1.0, 1.0, 1.0),
                  .skyBottom = glm::vec3(1.0, 1.0, 1.0),
                  .fogColor = glm::vec3(0.4, 0.7, 1.0) };
            case PlanetType::Icy:
                  return Atmosphere {
                  .skyTop = glm::vec3(1.0, 1.0, 1.0),
                  .skyBottom = glm::vec3(1.0, 1.0, 1.0),
                  .fogColor = glm::vec3(0.4, 0.7, 1.0) };
            case PlanetType::Terriproge:
                  return Atmosphere {
                  .skyTop = glm::vec3(1.0, 1.0, 1.0),
                  .skyBottom = glm::vec3(1.0, 1.0, 1.0),
                  .fogColor = glm::vec3(0.4, 0.7, 1.0) };
      }
      return Atmosphere {
            .skyTop = glm::vec3(1.0, 1.0, 1.0),
            .skyBottom = glm::vec3(1.0, 1.0, 1.0),
            .fogColor = glm::vec3(0.4, 0.7, 1.0) };
}



#endif //PLANETS_H
