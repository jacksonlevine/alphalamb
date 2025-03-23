//
// Created by jack on 3/23/2025.
//

#include "DrawSky.h"

#include "Planets.h"
#include "PrecompHeader.h"
#include "Scene.h"
#include "Sky.h"
#include "TimeOfDay.h"


void dgDrawSky(const glm::vec3& pos, GLuint lutTexture, World& world, float timeOfDay) {
        Atmosphere currAtmos = skyAndFogColor(theScene.currentPlanetType);

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
                ambBrightFromTimeOfDay(timeOfDay, theScene.dayLength), theScene.guiCamera,
                lutTexture, currAtmos.fogColor, dewyFogFactorAtCam);
}
