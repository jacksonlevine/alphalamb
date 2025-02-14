//
// Created by jack on 1/28/2025.
//

#include "BlockSelectGizmo.h"

#include "../../PhysXStuff.h"
#include "../../Shader.h"
GLuint BlockSelectGizmo::indices[] = {
    0, 1,   1, 2,   2, 3,   3, 0,  // Front face
   4, 5,   5, 6,   6, 7,   7, 4,  // Back face
   0, 4,   1, 5,   2, 6,   3, 7   // Connecting lines
};
void BlockSelectGizmo::draw(World* world, Player* player)
{

    glBindVertexArray(vao);
    glUseProgram(shaderProgram);

    constexpr int DISTANCE = 10;


    glm::vec3 direction = player->camera.transform.direction;
    glm::vec3 position = player->camera.transform.position;
    glLineWidth(3.0);

    PxRaycastBuffer hit;
    PxQueryFilterData fd;
    fd.flags |= PxQueryFlag::eSTATIC; // Ensure it checks both static & dynamic objects

    // Exclude objects with word0 = 2
    fd.data.word0 = ~2;  // This ensures that objects with word0 = 2 are ignored

    bool isHit = gScene->raycast(PxVec3(position.x, position.y, position.z),
                                 PxVec3(direction.x, direction.y, direction.z),
                                 DISTANCE, hit, PxHitFlag::eDEFAULT, fd);

    isDrawing = false;
    if(isHit)
    {
        auto hitCount = hit.getNbAnyHits();


        glm::vec3 spot(0,0,0);

        bool draw = false;
        for(int i = 0; i < hitCount; i++)
        {
            auto h = hit.getAnyHit(i);
            if (h.actor->getType() == PxActorType::eRIGID_STATIC)
            {
                draw = true;
                // std::cout << " Hit at position " << h.position.x << " " << h.position.y  << " " << h.position.z << " \n";
                PxVec3 pos = h.position;

                //Always go inside the block
                pos += (h.normal * -1.0) * 0.5;

                float epsilon = 0.0001f;
                spot = glm::vec3(std::floor(pos.x + epsilon), std::floor(pos.y + epsilon), std::floor(pos.z + epsilon));
                selectedSpot = IntTup(spot.x, spot.y, spot.z);
                hitNormal = IntTup(h.normal.x, h.normal.y, h.normal.z);
            }
        }

        if (draw)
        {
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE,
            glm::value_ptr(player->camera.mvp));
            glUniform3f(glGetUniformLocation(shaderProgram, "pos"),
                spot.x, spot.y, spot.z);

            glDrawElements(GL_LINES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
            isDrawing = true;
        }

    }

}

void BlockSelectGizmo::init()
{
    // Vertex data for a cube (0.0 to 1.0)
    GLfloat vertices[] = {
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
        float checkerboard = mod(floor(gl_FragCoord.x/10.0) + floor(gl_FragCoord.y/10.0), 2.0);
        vec3 color = mix(vec3(0.0), vec3(1.0), checkerboard);
        FragColor = vec4(color.xyz, 1.0);
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
