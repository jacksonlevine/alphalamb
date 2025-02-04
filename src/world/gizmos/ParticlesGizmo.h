//
// Created by jack on 2/3/2025.
//

#ifndef PARTICLESGIZMO_H
#define PARTICLESGIZMO_H

#include "../WorldGizmo.h"


class ParticlesGizmo : public WorldGizmo {
public:
    void draw(World* world, Player* player) const override;
    void init() override;
private:
    GLuint vao = 0;
    GLuint mainuvvbo, mainvbo = 0;

    GLuint instancesvao = 0;
    GLuint shaderProgram = 0;

};


#endif //PARTICLESGIZMO_H
