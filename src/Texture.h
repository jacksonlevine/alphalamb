//
// Created by jack on 1/10/2025.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include "PrecompHeader.h"

namespace jl
{

    class Texture {
    public:
        GLuint id;
        glm::ivec2 size;

        Texture(const std::string& filepath);  // Constructor to load texture from file
        void bind_to_unit(GLuint unit) const;  // Bind texture to specified unit
        ~Texture();                            // Destructor to clean up texture

    private:
        //std::vector<unsigned char> data;       // Image data storage
    };

}




#endif //TEXTURE_H
