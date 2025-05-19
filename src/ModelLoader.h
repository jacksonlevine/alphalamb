//
// Created by jack on 10/13/2024.
//

#ifndef MODELLOADER_H
#define MODELLOADER_H

#include "PrecompHeader.h"
#include "PhysXStuff.h"

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
    GLuint nbo; // only loaded to if INCLUDENORMALS true
}ModelGLObjects;

typedef struct
{
    std::vector<ModelGLObjects> modelGLObjects;
    std::vector<GLuint> texids;
    physx::PxRigidStatic* rigidStatic;
}ModelAndTextures;



GLenum getGLTypeFromTinyGLTFComponentType(int componentType);
int getNumberOfComponents(int acctype);


template <bool INCLUDENORMALS = false>
ModelAndTextures loadModel(const char* path, bool createCollider)
{




    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    //const bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path); // for .gltf
    const bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path); // for .glb

    if (!warn.empty()) {

    }

    if (!err.empty()) {

    }

    if (!ret) {

    }

    std::vector<ModelGLObjects> modelGLObjects;
    std::vector<GLuint> texids;




    std::vector<PxVec3> collpositions;
    std::vector<PxU32> collindices;


for (const tinygltf::Mesh& mesh : model.meshes) {
    int index = 0;


    for (const tinygltf::Primitive& primitive : mesh.primitives) {

        if (index == 0)
        {
            // === Print Vertex Positions ===
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                const tinygltf::Accessor& positionAccessor = model.accessors[primitive.attributes.at("POSITION")];
                const tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];
                const tinygltf::Buffer& positionBuffer = model.buffers[positionBufferView.buffer];


                const float* positions = reinterpret_cast<const float*>(&positionBuffer.data.at(0) + positionBufferView.byteOffset);
                for (size_t i = 0; i < positionAccessor.count; ++i) {
                    int components = getNumberOfComponents(positionAccessor.type);
                    collpositions.push_back(PxVec3(positions[i * components + 0], positions[i * components + 1], positions[i * components + 2]));

                }
            }

            if constexpr (INCLUDENORMALS)
            {
                // === Print Vertex Normals ===
                if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                    const tinygltf::Accessor& normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
                    const tinygltf::BufferView& normalBufferView = model.bufferViews[normalAccessor.bufferView];
                    const tinygltf::Buffer& normalBuffer = model.buffers[normalBufferView.buffer];

                    const float* normals = reinterpret_cast<const float*>(&normalBuffer.data.at(0) + normalBufferView.byteOffset);
                    std::cout << "Normals (Count: " << normalAccessor.count << "):" << std::endl;
                    for (size_t i = 0; i < normalAccessor.count; ++i) {
                        int components = getNumberOfComponents(normalAccessor.type);
                        std::cout << "Normal " << i << ": ("
                                  << normals[i * components + 0] << ", "
                                  << normals[i * components + 1] << ", "
                                  << normals[i * components + 2] << ")" << std::endl;
                    }
                }
            }


            // === Print Indices ===
            if (primitive.indices >= 0) {
                const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];

                //std::cout << "Indices (Count: " << indexAccessor.count << "):" << std::endl;
                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    // std::cout << "UNSIGNED BYTE\n";
                    // const uint8_t* indices = reinterpret_cast<const uint8_t*>(&indexBuffer.data.at(0) + indexBufferView.byteOffset);
                    // for (size_t i = 0; i < indexAccessor.count; ++i) {
                    //     std::cout << static_cast<uint32_t>(indices[i]) << " ";
                    // }
                } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {

                    const uint16_t* indices = reinterpret_cast<const uint16_t*>(&indexBuffer.data.at(0) + indexBufferView.byteOffset);
                    for (size_t i = 0; i < indexAccessor.count; ++i) {
                        collindices.push_back(indices[i]);
                    }
                } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    // std::cout << "UNSIGNED NIT \n";
                    // const uint32_t* indices = reinterpret_cast<const uint32_t*>(&indexBuffer.data.at(0) + indexBufferView.byteOffset);
                    // for (size_t i = 0; i < indexAccessor.count; ++i) {
                    //     std::cout << indices[i] << " ";
                    // }
                }

            }
        }
        index++;
    }


}



    for (const tinygltf::Mesh& mesh : model.meshes) {
        for (const tinygltf::Primitive& primitive : mesh.primitives) {
            GLuint vao, vbo, ebo, texvbo;
            GLuint nbo = 0;

            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &texvbo);
            glGenBuffers(1, &ebo);
            if constexpr (INCLUDENORMALS)
            {
                glGenBuffers(1, &nbo);
            }


            glBindVertexArray(vao);
            //Indent operations on this vertex array object
                //Vertex positions buffer
                const tinygltf::Accessor& positionAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];
                const tinygltf::Buffer& positionBuffer = model.buffers[positionBufferView.buffer];
                glBindBuffer(positionBufferView.target, vbo);
                glBufferData(positionBufferView.target, positionBufferView.byteLength,
                         &positionBuffer.data.at(0) + positionBufferView.byteOffset,
                         GL_STATIC_DRAW);
                glVertexAttribPointer(0, getNumberOfComponents(positionAccessor.type), positionAccessor.componentType, positionAccessor.normalized ? GL_TRUE : GL_FALSE, positionAccessor.ByteStride(positionBufferView), nullptr);
                glEnableVertexAttribArray(0);

            if constexpr (INCLUDENORMALS)
            {
                const tinygltf::Accessor& normalAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                const tinygltf::BufferView& normalBufferView = model.bufferViews[normalAccessor.bufferView];
                const tinygltf::Buffer& normalBuffer = model.buffers[normalBufferView.buffer];
                glBindBuffer(normalBufferView.target, nbo);

                glBufferData(normalBufferView.target, normalBufferView.byteLength,
                         &normalBuffer.data.at(0) + normalBufferView.byteOffset,
                         GL_STATIC_DRAW);
                glVertexAttribPointer(4, getNumberOfComponents(normalAccessor.type), normalAccessor.componentType, normalAccessor.normalized ? GL_TRUE : GL_FALSE, normalAccessor.ByteStride(normalBufferView), nullptr);
                glEnableVertexAttribArray(4);
            }




                //Texture UVs buffer
                const tinygltf::Accessor& texCoordAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& texCoordBufferView = model.bufferViews[texCoordAccessor.bufferView];
                const tinygltf::Buffer& texCoordBuffer = model.buffers[texCoordBufferView.buffer];
                glBindBuffer(texCoordBufferView.target, texvbo);
                glBufferData(texCoordBufferView.target, texCoordBufferView.byteLength,
                         &texCoordBuffer.data.at(0) + texCoordBufferView.byteOffset,
                         GL_STATIC_DRAW);
                glVertexAttribPointer(1, getNumberOfComponents(texCoordAccessor.type), texCoordAccessor.componentType, texCoordAccessor.normalized ? GL_TRUE : GL_FALSE, texCoordAccessor.ByteStride(texCoordBufferView), nullptr);
                glEnableVertexAttribArray(1);

                //Indices buffer
                const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];
                glBindBuffer(indexBufferView.target, ebo);
                glBufferData(indexBufferView.target, indexBufferView.byteLength,
                         &indexBuffer.data.at(0) + indexBufferView.byteOffset,
                         GL_STATIC_DRAW);
                //Leave the vao with the index buffer bound, ready to go for drawElements
            glBindVertexArray(0);


            const GLenum indextype = getGLTypeFromTinyGLTFComponentType(indexAccessor.componentType); //We need this when we draw it too for some reason, so holding on to it

            modelGLObjects.emplace_back(ModelGLObjects{vbo,
                texvbo,
                vao,
                ebo,
                static_cast<uint32_t>(indexAccessor.count),
                static_cast<GLenum>(primitive.mode),
                indextype,
                nbo
            });
        }
    }

    for (const auto& texture : model.textures) {

        const tinygltf::Image& image = model.images[texture.source];

        GLuint texid;
        glGenTextures(1, &texid);

        glBindTexture(GL_TEXTURE_2D, texid);

        GLenum pixtype = getGLTypeFromTinyGLTFComponentType(image.pixel_type);

        GLenum format = GL_RGB;
        if (image.component == 1) {
            format = GL_RED;
        } else if (image.component == 3) {
            format = GL_RGB;
        } else if (image.component == 4) {
            format = GL_RGBA;
        }

        glTexImage2D(
            GL_TEXTURE_2D,
            0,                // mipmap level
            format,           // internal format
            image.width,      // width
            image.height,     // height
            0,                // border (this must be 0 according to gl docs lol)
            format,           // format of the pixel data
            pixtype,          // type of the pixel data
            &image.image[0]   // pointer to the actual pixel data
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        texids.push_back(texid);
    }

    if (createCollider)
    {
        if (path == "assets/models/ladder1.glb")
        {

            auto myrig = _createStaticMeshCollider(PxVec3(0.0, 0.0, 0.0), collpositions, collindices, true);
            return ModelAndTextures{modelGLObjects, texids, myrig};
        } else
        {
            auto myrig = createStaticMeshCollider(PxVec3(0.0, 0.0, 0.0), collpositions, collindices);
            return ModelAndTextures{modelGLObjects, texids, myrig};
        }

    } else
    {
        return ModelAndTextures{modelGLObjects, texids, nullptr};
    }


}

} // jl

#endif //MODELLOADER_H
