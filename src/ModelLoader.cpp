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



ModelAndTextures ModelLoader::loadModel(const char* path, bool createCollider)
{

    std::cout << "Loading Model File: " << path << std::endl;


    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    //const bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path); // for .gltf
    const bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path); // for .glb

    if (!warn.empty()) {
        std::cout << "Warn: " << warn.c_str() << "\n";
    }

    if (!err.empty()) {
        std::cout << "Err: " << err.c_str() << "\n";
    }

    if (!ret) {
        std::cout << "Failed to parse glTF at " << path << "\n";
    }

    std::vector<ModelGLObjects> modelGLObjects;
    std::vector<GLuint> texids;




    std::vector<PxVec3> collpositions;
    std::vector<PxU32> collindices;


for (const tinygltf::Mesh& mesh : model.meshes) {
    int index = 0;

        std::cout << "Mesh \n";
    for (const tinygltf::Primitive& primitive : mesh.primitives) {
        std::cout << "Primitive \n";
        if (index == 0)
        {
            // === Print Vertex Positions ===
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                const tinygltf::Accessor& positionAccessor = model.accessors[primitive.attributes.at("POSITION")];
                const tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];
                const tinygltf::Buffer& positionBuffer = model.buffers[positionBufferView.buffer];

                std::cout << "Vertex Positions (Count: " << positionAccessor.count << "):" << std::endl;
                const float* positions = reinterpret_cast<const float*>(&positionBuffer.data.at(0) + positionBufferView.byteOffset);
                for (size_t i = 0; i < positionAccessor.count; ++i) {
                    int components = getNumberOfComponents(positionAccessor.type);
                    collpositions.push_back(PxVec3(positions[i * components + 0], positions[i * components + 1], positions[i * components + 2]));

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
                    std::cout << "UnSIGNED SHORT \n";
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
                std::cout << std::endl;
            }
        }
        index++;
    }


}



    for (const tinygltf::Mesh& mesh : model.meshes) {
        for (const tinygltf::Primitive& primitive : mesh.primitives) {
            GLuint vao, vbo, ebo, texvbo;

            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &texvbo);
            glGenBuffers(1, &ebo);

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
                indextype
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
            std::cout <<"Ladder \n";
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