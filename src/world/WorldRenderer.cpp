//
// Created by jack on 1/27/2025.
//

#include "WorldRenderer.h"


void modifyOrInitializeDrawInstructions(GLuint& vvbo, GLuint& uvvbo, GLuint& ebo, DrawInstructions& drawInstructions, UsableMesh& usable_mesh, GLuint& bvbo)
{
    if(drawInstructions.vao == 0)
    {

        glGenVertexArrays(1, &drawInstructions.vao);
        glBindVertexArray(drawInstructions.vao);
        glGenBuffers(1, &vvbo);
        glGenBuffers(1, &uvvbo);
        glGenBuffers(1, &ebo);
        glGenBuffers(1, &bvbo);
    } else
    {
        glBindVertexArray(drawInstructions.vao);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.positions) * sizeof(PxVec3), usable_mesh.positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, bvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.brightness) * sizeof(float), usable_mesh.brightness.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);


    glBindBuffer(GL_ARRAY_BUFFER, uvvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.texcoords) * sizeof(glm::vec2), usable_mesh.texcoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, std::size(usable_mesh.indices) * sizeof(PxU32), usable_mesh.indices.data(), GL_STATIC_DRAW);

    drawInstructions.indiceCount = std::size(usable_mesh.indices);
}

///This assumes shader is set up and uniforms are given values
void drawFromDrawInstructions(const DrawInstructions& drawInstructions)
{
    glBindVertexArray(drawInstructions.vao);
    glDrawElements(GL_TRIANGLES, drawInstructions.indiceCount, GL_UNSIGNED_INT, 0);
}

void WorldRenderer::mainThreadDraw()
{

    for (const auto & [spot, index] : activeChunks)
    {
        auto glInfo = chunkPool[index];
        drawFromDrawInstructions(glInfo.drawInstructions);
    }
}

void WorldRenderer::meshBuildCoroutine(jl::Camera* playerCamera)
{
}
