//
// Created by jack on 1/28/2025.
//

#ifndef BLOCKSELECTGIZMO_H
#define BLOCKSELECTGIZMO_H

#include "../WorldGizmo.h"

constexpr float BLOCK_BREAK_TIME = 0.01f;

class BlockSelectGizmo : public WorldGizmo {
public:
    void draw(World* world, entt::entity playerIndex, entt::registry& reg) override;
    void init() override;
    IntTup selectedSpot = {};
    IntTup hitNormal ={};
    bool isDrawing = false;

    float hitProgress = 0.0f;
    bool hitting = false;



private:
    GLuint vao = 0, vbo = 0, ebo = 0;
    GLuint overlayvao = 0, overlayvbo = 0, overlayebo = 0, overlayuvvbo = 0;
    GLuint shaderProgram = 0;
    GLuint overlayShaderProgram = 0;
    // Indices for drawing the cube using lines
    static GLuint indices[];
    static GLuint overlayIndices[];
};



#endif //BLOCKSELECTGIZMO_H
