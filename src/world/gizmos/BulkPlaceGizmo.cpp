//
// Created by jack on 2/11/2025.
//

#include "BulkPlaceGizmo.h"

GLuint BulkPlaceGizmo::indices[] = {
    0, 1,   1, 2,   2, 3,   3, 0,  // Front face
   4, 5,   5, 6,   6, 7,   7, 4,  // Back face
   0, 4,   1, 5,   2, 6,   3, 7   // Connecting lines
};

void BulkPlaceGizmo::draw(World* world, Player* player)
{
    static IntTup lastcorner1 = corner1;
    static IntTup lastcorner2 = corner2;


    static GLfloat vertices[] = {
        // Front face vertices
        -0.01f, -0.01f, -0.01f,  // 0
        1.01f, -0.01f, -0.01f,  // 1
        1.01f, 1.01f, -0.01f,  // 2
        -0.01f, 1.01f, -0.01f,  // 3
        // Back face vertices
        -0.01f, -0.01f, 1.01f,  // 4
        1.01f, -0.01f, 1.01f,  // 5
        1.01f, 1.01f, 1.01f,  // 6
        -0.01f, 1.01f, 1.01f   // 7
    };



    if (lastcorner1 != corner1 || lastcorner2 != corner2)
    {
        glBindVertexArray(vao);
        std::vector<float> verts = {};
        verts.reserve(24);
        verts.insert(verts.end(), {
            vertices[0] + corner1.x, vertices[1] + corner1.y, vertices[2] + corner1.z,
            vertices[3] + corner2.x, vertices[4] + corner1.y, vertices[5] + corner1.z,
            vertices[6] + corner2.x, vertices[7] + corner2.y, vertices[8] + corner1.z,
            vertices[9] + corner1.x, vertices[10] + corner2.y, vertices[11] + corner1.z,

            vertices[12] + corner1.x, vertices[13] + corner1.y, vertices[14] + corner2.z,
            vertices[15] + corner2.x, vertices[16] + corner1.y, vertices[17] + corner2.z,
            vertices[18] + corner2.x, vertices[19] + corner2.y, vertices[20] + corner2.z,
            vertices[21] + corner1.x, vertices[22] + corner2.y, vertices[23] + corner2.z,
        });
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    }


    if (active)
    {
        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE,
        glm::value_ptr(player->camera.mvp));
        glUniform3f(glGetUniformLocation(shaderProgram, "pos"),
            0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(shaderProgram, "placemode"), placeMode);

        glDrawElements(GL_LINES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
    }
}

void BulkPlaceGizmo::init()
{

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
    uniform float placemode;
    void main()
    {
        float checkerboard = mod(floor(gl_FragCoord.x/10.0) + floor(gl_FragCoord.y/10.0), 2.0);

        vec3 c = vec3(1.0);
        if(placemode == 0.0) {
            c = vec3(1.0, 0.0, 0.0);
        } else {
            c = vec3(1.0, 1.0, 0.0);
        }
        vec3 color = mix(vec3(0.0), c, checkerboard);
        FragColor = vec4(color.xyz, 1.0);
    }
    )glsl",
        "bulkplaceshader");

    shaderProgram = shader.shaderID;

    // Set up VAO and VBO
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, nullptr, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}
