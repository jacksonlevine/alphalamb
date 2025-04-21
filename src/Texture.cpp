//
// Created by jack on 1/10/2025.
//

#define TINYGLTF_IMPLEMENTATION

#include "Texture.h"


namespace jl
{

    Texture::Texture(const std::string& filepath) {
        int width, height, channels;
        id = 0;

        // Load the image using stb_image
        unsigned char* imgData = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!imgData) {
            std::cerr << "Failed to load texture: " << filepath << std::endl;
            return;
        }

        size = glm::ivec2(width, height);
        //data.assign(imgData, imgData + width * height * 4);


        // Generate OpenGL texture
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        // Set texture parameters
  /*      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);*/

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Upload texture data to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData);

        //glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        stbi_image_free(imgData);
    }

    void Texture::bind_to_unit(const GLuint unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    Texture::~Texture() {
        //glDeleteTextures(1, &id);
    }

}
