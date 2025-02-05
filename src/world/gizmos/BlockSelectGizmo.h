//
// Created by jack on 1/28/2025.
//

#ifndef BLOCKSELECTGIZMO_H
#define BLOCKSELECTGIZMO_H

#include "../WorldGizmo.h"

class BlockSelectGizmo : public WorldGizmo {
public:
    void draw(World* world, Player* player) override;
    void init() override;
private:
    GLuint vao, vbo, ebo = 0;
    GLuint shaderProgram = 0;
    // Indices for drawing the cube using lines
    static GLuint indices[];
};



#endif //BLOCKSELECTGIZMO_H
