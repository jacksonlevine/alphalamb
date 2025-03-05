//
// Created by jack on 10/13/2024.
//

#ifndef SHADER_H
#define SHADER_H


#include "PrecompHeader.h"

namespace jl {

class Shader {
public:
    GLuint shaderID;
    Shader(const char *vertex, const char *frag, const char *name);
};

}




#endif //SHADER_H
