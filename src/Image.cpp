//
// Created by jack on 2/8/2025.
//

#include "Image.h"




namespace jl
{

    Image::Image(const std::string& filepath) {
        // Load the image using stb_image
        unsigned char* imgData = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!imgData) {
            std::cerr << "Failed to load Image: " << filepath << std::endl;
            return;
        }

        data.assign(imgData, imgData + width * height * channels);
        stbi_image_free(imgData);
    }


}
