//
// Created by jack on 2/4/2025.
//

#ifndef SWCLOADER_H
#define SWCLOADER_H

#include "VoxelModels.h"
#include "World.h"


inline VoxModel loadSwc(std::string filepath)
{
    std::fstream file(filepath);
    std::string line;

    VoxModel model = {};
    model.dimensions = IntTup(10000,10000,250);


    while(std::getline(file, line))
    {
        std::istringstream linestream(line);
        std::vector<std::string> words;
        std::string w;

        while(linestream >> w) {
            words.push_back(w);
        }
        if(!words.empty())
        {
            if(words.at(0).starts_with('#'))
                continue;

            model.points.push_back(VoxelPoint{
                .localSpot = IntTup((std::stof(words.at(2)) - 20000) / 100,
                            (std::stof(words.at(3)) - 20000) / 100,
                            (std::stof(words.at(4)) - 20000) / 100),
                .colorIndex = (uint8_t)(5+std::stoi(words[1]))
            });

            //std::cout << "Coordinate: " << model.points.back().localSpot.x << " " <<   model.points.back().localSpot.y << " " <<  model.points.back().localSpot.z << "\n";
        }


    }

    return model;
}

inline void stampVoxelModelInWorld(World* world, VoxModel& model)
{
    for(auto & point : model.points)
    {
        world->nonUserDataMap->set(point.localSpot, point.colorIndex);
    }
}


#endif //SWCLOADER_H
