//
// Created by jack on 2/11/2025.
//

#ifndef BULKPLACEGIZMO_H
#define BULKPLACEGIZMO_H

#include "../WorldGizmo.h"



class BulkPlaceGizmo : public WorldGizmo  {
public:
    void draw(World* world, Player* player) override;
    void init() override;

    bool active = false;

    IntTup corner1 = {};
    IntTup corner2 = {};

    enum PlaceMode
    {
        Solid,
        Walls
    };
    PlaceMode placeMode = Solid;

    GLuint vao, vbo, ebo = 0;
    GLuint shaderProgram = 0;

    static GLuint indices[];
};



#endif //BULKPLACEGIZMO_H
