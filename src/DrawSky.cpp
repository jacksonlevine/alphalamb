//
// Created by jack on 3/23/2025.
//

#include "DrawSky.h"

#include "Planets.h"
#include "PrecompHeader.h"
#include "Scene.h"
#include "Sky.h"
#include "TimeOfDay.h"


void dgDrawSky(const glm::vec3& pos, GLuint lutTexture, World& world, float timeOfDay, jl::Camera* cam) {
        Atmosphere currAtmos = skyAndFogColor(theScene.currentPlanetType);
        auto ambb = ambBrightFromTimeOfDay(timeOfDay, theScene.dayLength);
        if (liquids.test(theScene.blockHeadIn)) {
            currAtmos.skyTop = glm::vec3(0.f, 0.f, 1.f) * ambb;
            currAtmos.skyBottom = glm::vec3(0.f, 0.f, .2f) * ambb;
        }

        IntTup itspot(
            pos.x,
            pos.y,
            pos.z
        );

        float temperature_noise = world.worldGenMethod->getTemperatureNoise(itspot);
        float humidity_noise = world.worldGenMethod->getHumidityNoise(itspot);
        float dewyFogFactorAtCam = theScene.worldRenderer->getDewyFogFactor(temperature_noise, humidity_noise);

        drawSky(glm::vec4(currAtmos.skyTop, 1.0),
                glm::vec4(currAtmos.skyBottom, 1.0),
                ambb, cam,
                lutTexture, currAtmos.fogColor, dewyFogFactorAtCam);
}
