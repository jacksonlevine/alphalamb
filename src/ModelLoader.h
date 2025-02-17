//
// Created by jack on 10/13/2024.
//

#ifndef MODELLOADER_H
#define MODELLOADER_H

#include "PrecompHeader.h"

namespace jl {

typedef struct
{
    GLuint vbo;
    GLuint texvbo;
    GLuint vao;
    GLuint ebo;
    uint32_t indexcount;
    GLenum drawmode;
    GLenum indextype;
}ModelGLObjects;

typedef struct
{
    std::vector<ModelGLObjects> modelGLObjects;
    std::vector<GLuint> texids;
    physx::PxRigidStatic* rigidStatic;
}ModelAndTextures;

class ModelLoader {
public:
    static ModelAndTextures loadModel(const char* path, bool createCollider);
};

} // jl

#endif //MODELLOADER_H
