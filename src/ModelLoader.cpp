//
// Created by jack on 10/13/2024.
//

#include "ModelLoader.h"
#include <cassert>

#include "PhysXStuff.h"


namespace jl {

GLenum getGLTypeFromTinyGLTFComponentType(int componentType) {
    switch (componentType) {
    case TINYGLTF_COMPONENT_TYPE_BYTE: return GL_BYTE;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
    case TINYGLTF_COMPONENT_TYPE_SHORT: return GL_SHORT;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return GL_UNSIGNED_SHORT;
    case TINYGLTF_COMPONENT_TYPE_INT: return GL_INT;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return GL_UNSIGNED_INT;
    case TINYGLTF_COMPONENT_TYPE_FLOAT: return GL_FLOAT;
    default: return GL_FLOAT;
    }
}

int getNumberOfComponents(int acctype)
{
    int size = 1;
    if (acctype == TINYGLTF_TYPE_SCALAR) {
        size = 1;
    } else if (acctype == TINYGLTF_TYPE_VEC2) {
        size = 2;
    } else if (acctype == TINYGLTF_TYPE_VEC3) {
        size = 3;
    } else if (acctype == TINYGLTF_TYPE_VEC4) {
        size = 4;
    } else {
        assert(0);
    }
    return size;
}




} // jl