//
// Created by jack on 11/30/2024.
//

#ifndef HUD_H
#define HUD_H

#include "PrecompHeader.h"

struct Hud {
    GLuint vbo;
    GLuint texture;
    std::vector<float> displayData;
    bool uploaded = false;
    inline static int windowWidth = 1280;
    inline static int windowHeight = 720;
    Hud();
    void rebuildDisplayData();
    void draw();
};



#endif //HUD_H
