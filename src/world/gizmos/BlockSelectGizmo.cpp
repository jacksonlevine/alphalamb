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
    glLineWidth(4.0);


    PxRaycastBuffer hit;
    PxQueryFilterData fd;
    fd.flags |= PxQueryFlag::eANY_HIT;

    bool isHit = gScene->raycast(PxVec3(position.x, position.y, position.z), PxVec3(direction.x, direction.y, direction.z),
        DISTANCE, hit, PxHitFlags(PxHitFlag::eDEFAULT), fd);
    //std::cout << "Casting a ray: \n";

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
                pos += (h.normal * -1.0) * 0.3;

                spot = glm::vec3(std::floor(pos.x), std::floor(pos.y), std::floor(pos.z));
            }


        }

        if (draw)
        {
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE,
            glm::value_ptr(player->camera.mvp));
            glUniform3f(glGetUniformLocation(shaderProgram, "pos"),
                spot.x, spot.y, spot.z);

            glDrawElements(GL_LINES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
        }

    }

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
