//
// Created by jack on 1/28/2025.
//

#include "BlockSelectGizmo.h"

#include "../../Shader.h"
GLuint BlockSelectGizmo::indices[] = {
    0, 1,   1, 2,   2, 3,   3, 0,  // Front face
   4, 5,   5, 6,   6, 7,   7, 4,  // Back face
   0, 4,   1, 5,   2, 6,   3, 7   // Connecting lines
};
void BlockSelectGizmo::draw(World* world, Player* player) const
{
    glBindVertexArray(vao);
    glUseProgram(shaderProgram);
    glm::vec3 spot = player->camera.transform.position + player->camera.transform.direction * 3.0f;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE,
        glm::value_ptr(player->camera.mvp));
    glUniform3f(glGetUniformLocation(shaderProgram, "pos"),
        spot.x, spot.y, spot.z);
    glDrawElements(GL_LINES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}

void BlockSelectGizmo::init()
{
    // Vertex data for a cube (0.0 to 1.0)
    GLfloat vertices[] = {
        // Front face vertices
        0.0f, 0.0f, 0.0f,  // 0
        1.0f, 0.0f, 0.0f,  // 1
        1.0f, 1.0f, 0.0f,  // 2
        0.0f, 1.0f, 0.0f,  // 3
        // Back face vertices
        0.0f, 0.0f, 1.0f,  // 4
        1.0f, 0.0f, 1.0f,  // 5
        1.0f, 1.0f, 1.0f,  // 6
        0.0f, 1.0f, 1.0f   // 7
    };

    jl::Shader shader(R"glsl(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    uniform vec3 pos;
    uniform mat4 mvp;
    void main()
    {
        gl_Position = mvp * vec4(aPos + pos, 1.0);
    }
    )glsl",
        R"glsl(
    #version 330 core
    out vec4 FragColor;
    void main()
    {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    )glsl",
        "blockSelectGizmoShader");

    shaderProgram = shader.shaderID;

    // Set up VAO and VBO
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}
