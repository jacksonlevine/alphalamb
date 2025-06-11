//
// Created by jack on 1/27/2025.
//

#include "OverworldWorldGenMethod.h"
#include "../MaterialName.h"

std::vector<std::vector<VoxelModelName>> terrainFeatureModels = {
    {Awds},
    {PineTree1, PineTree2},
    {ArticWillow},
    {Tree1, Tree2},
    {Tree3, Tree4},
    {CedarTree1, CedarTree2},
    {PalmTree1, PalmTree2, PalmTree3},
    {JoshuaTree1, JoshuaTree2, JoshuaTree3},
    {RubberTree},
    {PaperBirch1, PaperBirch2, PaperBirch3},
    {GreenAlder1, GreenAlder2},
    {Willow1, Willow2, Willow3},
    {Beech1, Beech2, Beech3},
    {WesternHemlock1, WesternHemlock2, WesternHemlock3},
    {Eucalyptus1, Eucalyptus2, Eucalyptus3},
    {Saguaro1, Saguaro2, Saguaro3},
    {FigTree1, FigTree2},
    {Pumpkin},
    {Rock1, Rock2},
    {Bamboo1, Bamboo2},
    {Bush}
};

std::vector<std::vector<TerrainFeature>> climateTerrainFeatures = {
    {Boulder, ArticWillowDwarfShrub},
    {LightberryBush, Boulder, Pine, PaperBirch},
    {Boulder, ArticWillowFeat, GreenAlder},
    {LightberryBush, Oak, Willow, PumpkinFeat},
    {LightberryBush, Oak, Maple, Beech},
    {LightberryBush, Bamboo, Boulder, Maple, Cedar, WesternHemlock},
    {Joshua, Saguaro, Boulder},
    {Palm, Eucalyptus},
    {Bamboo, Rubber, FigTree}
};

OverworldWorldGenMethod::OverworldWorldGenMethod()
{
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    srand(time(NULL));
    noise.SetSeed(rand());

    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(2);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFractalGain(0.5f);
}

BlockType OverworldWorldGenMethod::get(IntTup spot)
{

    const MaterialName floorBlock = getFloorBlockInClimate(getClimate(spot));

    static const MaterialName underDirt = COBBLESTONE;
    static const MaterialName surface = floorBlock;
    static const MaterialName underSurface = DIRT;
    static const MaterialName liquid = WATER;
    static const MaterialName beach = SAND;

    static const float WL = 60.0f;

    const float distfromorigin = 1.0f - (glm::abs(spot.x + spot.z) / 8'000.f);

    const float notype = std::min(0.01f, std::max(0.003f, noise.GetNoise(
        spot.x * 0.25f,
        spot.z * 0.25f) * 0.2f));
    //
    // notype = 0.0f;

    static const float yoff = 60.0f;

    const float nobelow = distfromorigin * (getNoiseMix(
        spot.x * blockScaleInPerlin,
        (spot.y - 1) * blockScaleInPerlin,
        spot.z * blockScaleInPerlin)
    - ((spot.y - 1 - yoff) * notype));

    const float grassnoise1 = noise.GetNoise(spot.x * 50.5f, spot.z * 50.5f);
    const float grassnoise2 = noise.GetNoise(spot.x * 15.5f, spot.z * 15.5f);

    const float no = distfromorigin * (getNoiseMix(
        spot.x * blockScaleInPerlin,
        spot.y * blockScaleInPerlin,
        spot.z * blockScaleInPerlin)
    - ((spot.y - yoff) * notype));

    const float noabove = distfromorigin * (getNoiseMix(
        spot.x * blockScaleInPerlin,
        (spot.y + 1) * blockScaleInPerlin,
        spot.z * blockScaleInPerlin)
    - ((spot.y + 1 - yoff) * notype));

    const float no5up = distfromorigin * (getNoiseMix(
        spot.x * blockScaleInPerlin,
        (spot.y + 6) * blockScaleInPerlin,
        spot.z * blockScaleInPerlin)
    - ((spot.y + 6 - yoff) * notype));

    const float no10up = distfromorigin * (getNoiseMix(
        spot.x * blockScaleInPerlin,
        (spot.y + 12) * blockScaleInPerlin,
        spot.z * blockScaleInPerlin)
    - ((spot.y + 12 - yoff) * notype));

    static const float THRESHOLD = 0.02f;

    if (no > THRESHOLD)
    {
        if (no10up > THRESHOLD)
        {
            return underDirt;
        } else
        {
            float beachNoise = noise.GetNoise(
                spot.x * beachNoiseScale,
                spot.y * beachNoiseScale,
                spot.z * beachNoiseScale);
            if (spot.y > (WL + (beachNoise*3.0f)) ||
                no5up > THRESHOLD)
            {
                if (noabove > THRESHOLD)
                {
                    return underSurface;
                } else
                {
                    return surface;
                }
            } else
            {
                return beach;
            }
        }
    } else
    {
        if (spot.y < WL)
        {
            return liquid;
        }
        if (nobelow > THRESHOLD && grassnoise1 > THRESHOLD && grassnoise2 > THRESHOLD)
        {
            return TALL_GRASS;
        }

        return AIR;


    }


}

Climate OverworldWorldGenMethod::getClimate(IntTup spot)
{
    const float tempNoise = getTemperatureNoise(spot);

    const float humidityNoise = getHumidityNoise(spot);

    const float tempClamped = glm::clamp(tempNoise, -1.0f, 1.0f);
    const float humClamped = glm::clamp(humidityNoise, -1.0f, 1.0f);

    const int tempIndex = std::min(2, static_cast<int>(std::floor((tempClamped + 1.0f) * 1.5f)));
    const int humIndex = std::min(2, static_cast<int>(std::floor((humClamped + 1.0f) * 1.5f)));

    const int index = tempIndex * 3 + humIndex;

    return static_cast<Climate>(index);
}

void OverworldWorldGenMethod::setSeed(int seed)
{
    noise.SetSeed(seed);
}

MaterialName OverworldWorldGenMethod::getFloorBlockInClimate(const Climate& climate)
{
    switch (climate)
    {
    case HotDesert:
        return SAND;
    case PolarDesert:
        return GRASS; //SNOWYGRASS
    default:
        return GRASS;
    }
}

float OverworldWorldGenMethod::getHumidityNoise(const IntTup& spot)
{
    return noise.GetNoise(
        spot.x * humidityNoiseScale,
        spot.y * humidityNoiseScale,
        spot.z * humidityNoiseScale)
    * 2.0 - ((spot.y - 60) / 500.0f);
}

float OverworldWorldGenMethod::getTemperatureNoise(const IntTup& spot)
{
    return noise.GetNoise(
        (spot.x + 1000) * tempNoiseScale,
        (spot.y + 1000) * tempNoiseScale,
        spot.z * tempNoiseScale)
    * 2.0 - ((spot.y - 60) / 500.0f);
}

float OverworldWorldGenMethod::getNoiseMix(float x, float y, float z)
{
    return (noise.GetNoise(x*0.1f,y-40,z*0.1f)*0.7);

}

int OverworldWorldGenMethod::getSeed()
{
    return noise.mSeed;
}
