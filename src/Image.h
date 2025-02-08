//
// Created by jack on 2/8/2025.
//

#ifndef IMAGE_H
#define IMAGE_H


#include "PrecompHeader.h"

namespace jl
{

    class Image {
    public:
        int width, height, channels;

        Image(const std::string& filepath);

        std::vector<unsigned char> data;
    };

}




#endif //IMAGE_H
