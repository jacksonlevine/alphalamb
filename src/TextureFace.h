#ifndef TEXTUREFACE_H
#define TEXTUREFACE_H
#pragma once

#include <glm/glm.hpp>

struct TextureFace {
public:
    glm::vec2 tl;
    glm::vec2 bl;
    glm::vec2 br;
    glm::vec2 tr;
    
    TextureFace(
        glm::vec2 tl,
        glm::vec2 bl,
        glm::vec2 br,
        glm::vec2 tr
    );
    TextureFace(
        int x,
        int y
    );
};


#endif