//
// Created by jack on 2/13/2025.
//

#ifndef VOXMODELSTAMPGIZMO_H
#define VOXMODELSTAMPGIZMO_H

#include "../WorldGizmo.h"
#include "../../PrecompHeader.h"

class VoxModelStampGizmo : public WorldGizmo {
public:
    void draw(World* world, Player* player) override;
    void init() override;
    ChunkGLInfo cgl = {};
    bool active = false;
    size_t modelIndex = 5;
    IntTup spot = {};
    GLuint shaderProgram = 0;
};



#endif //VOXMODELSTAMPGIZMO_H
