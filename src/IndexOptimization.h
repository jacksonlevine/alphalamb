//
// Created by jack on 2/12/2025.
//

#ifndef INDEXOPTIMIZATION_H
#define INDEXOPTIMIZATION_H

constexpr GLuint startingChunkVbo = 1;


//
// struct ChunkGLInfo
// {
//     GLuint vvbo, uvvbo, bvbo, ebo = 0;
//     GLuint tvvbo, tuvvbo, tbvbo, tebo = 0;
//     DrawInstructions drawInstructions = {};
// };


class IndexOptimization
{
public:
    enum Vao
    {
        VAO,
        TVAO
    };
    enum Buffer
    {
        VVBO, UVVBO, BVBO, EBO,
        TVVBO, TUVVBO, TBVBO, TEBO
    };
};

constexpr GLuint vaoNameOfChunkIndex(int chunkIndex, IndexOptimization::Vao vao)
{
    return startingChunkVbo + ((GLuint)chunkIndex) * 2 + (GLuint)vao;
}

constexpr GLuint bufferNameOfChunkIndex(int chunkIndex, IndexOptimization::Buffer buffer)
{
    return startingChunkVbo + ((GLuint)chunkIndex) *8 + buffer;
}

#endif //INDEXOPTIMIZATION_H
