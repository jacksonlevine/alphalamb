//
// Created by jack on 3/23/2025.
//
#include "SunAndMoon.h"

#include "Scene.h"
#include "Texture.h"

void drawSunAndMoon(jl::Camera* camera, float timeOfDay, float dayLength)
{

    static jl::Texture sunAndMoonTex("resources/bodies.png");



    static jl::Shader shader(
      R"glsl(
            #version 330 core

            //base geo
            layout(location = 0) in vec3 vertexPosition;
            layout(location = 1) in float cornerID;

            //instance
            layout(location = 2) in vec3 instancePosition;
            layout(location = 3) in vec3 instanceRotation;
            layout(location = 4) in float pIndex;
            layout(location = 5) in float scale;

            out vec2 tcoord;

            uniform mat4 mvp;
            uniform vec3 camPos;
            uniform float time;

            uniform float rot;

            mat4 getRotationMatrix(float xrot, float yrot, float zrot) {
                mat4 Rx = mat4(1.0, 0.0, 0.0, 0.0,
                               0.0, cos(xrot), -sin(xrot), 0.0,
                               0.0, sin(xrot), cos(xrot), 0.0,
                               0.0, 0.0, 0.0, 1.0);

                mat4 Ry = mat4(cos(yrot), 0.0, sin(yrot), 0.0,
                               0.0, 1.0, 0.0, 0.0,
                               -sin(yrot), 0.0, cos(yrot), 0.0,
                               0.0, 0.0, 0.0, 1.0);

                mat4 Rz = mat4(cos(zrot), -sin(zrot), 0.0, 0.0,
                               sin(zrot), cos(zrot), 0.0, 0.0,
                               0.0, 0.0, 1.0, 0.0,
                               0.0, 0.0, 0.0, 1.0);
                return Rz * Ry * Rx;
            }

            void main() {
                mat4 instanceRot = getRotationMatrix(instanceRotation.x, instanceRotation.y, instanceRotation.z);
                mat4 globalRotation = getRotationMatrix(rot, 0.0, 0.0);

                vec3 instanceRotatedPosition = (instanceRot * vec4(instancePosition, 1.0)).xyz;

                vec3 rotatedInstancePosition = (globalRotation * vec4(instanceRotatedPosition, 1.0)).xyz;

                vec3 rotatedVertexPosition = (globalRotation * vec4(vertexPosition * scale, 1.0)).xyz;


                vec3 look = normalize(rotatedInstancePosition - camPos);
                vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), look));
                vec3 up = cross(look, right);

                vec3 billboardedPosition = rotatedInstancePosition + rotatedVertexPosition + camPos;

                float distance = pow(distance(instancePosition, camPos)/(5), 2)/5.0f;

                gl_Position = mvp * vec4(billboardedPosition, 1.0);

                vec2 baseUV = vec2(0.0 + (pIndex * 0.5), 0.0);

                if (cornerID == 0.0) {
                    tcoord = baseUV;
                } else if (cornerID == 1.0) {
                    tcoord = vec2(baseUV.x + 0.5f, baseUV.y);
                } else if (cornerID == 2.0) {
                    tcoord = vec2(baseUV.x + 0.5f, baseUV.y + 0.5);
                } else if (cornerID == 3.0) {
                    tcoord = vec2(baseUV.x, baseUV.y + 0.5);
                }
            }
        )glsl",
        R"glsl(
            #version 330 core
            in vec2 tcoord;
            out vec4 FragColor;
            uniform sampler2D ourTexture;

            void main()
            {
                vec4 texColor = texture(ourTexture, tcoord);
                FragColor = texColor;
                if(texColor.a < 0.1) {
                    discard;
                }
            }

        )glsl",
      "sunAndMoonShader");

    struct CelestialBody
    {
        glm::vec3 position;
        glm::vec3 rotation;
        float index;
        float scale;
    };
    // Assuming you have a vector or array of celestial bodies
    static std::vector<CelestialBody> positions = {
        CelestialBody{glm::vec3(0.0f, 2000.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 40.0f},
        CelestialBody{glm::vec3(0.0f, 2000.0f, 0.0f), glm::vec3(3.14159265358979323846264338327f, 0.0f, 0.0f), 1.0f, 20.0f},




    };


    static GLuint vao = 0;
    static GLuint vvbo = 0;
    static GLuint instvbo = 0;

    if(vao == 0)
    {
        for(int i = 0; i < 8000; i++)
        {
            glm::vec2 posit((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
            float x = posit.x * 4000.0f - 2000.0f;
            float z = posit.y * 4000.0f - 2000.0f;
            float distanceFromCenter = sqrt(x * x + z * z);
            float maxDistance = sqrt(2000.0f * 2000.0f + 2000.0f * 2000.0f); // Maximum possible distance from (0, 0)
            float y = 4000.0f * (1.0f - 1.0f * (distanceFromCenter / maxDistance)) * 0.4; // Y decreases as distance increases

            if (500.0f * ((float)rand() / (float)RAND_MAX) * distanceFromCenter < 20000)
            {
                positions.push_back(
                    CelestialBody{glm::vec3(x, y, z), glm::vec3(3.14159265358979323846264338327f, 0.0f, 0.0f), 0.0f, std::max(0.1f, (float)rand() / (float)RAND_MAX)}
                );
            }

        }

    }

    float halfsize = 5.0f;
    float vertices[32] = {
        -halfsize,  0.0f, halfsize, 3.0f,
        halfsize,  0.0f, halfsize, 2.0f,
        halfsize, 0.0f, -halfsize, 1.0f,
        -halfsize, 0.0f, -halfsize, 0.0f,

        -halfsize, 0.0f, -halfsize, 0.0f,
        halfsize, 0.0f, -halfsize, 1.0f,
        halfsize,  0.0f, halfsize, 2.0f,
        -halfsize,  0.0f, halfsize, 3.0f,

    };

    glUseProgram(shader.shaderID);

    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vvbo);
        glGenBuffers(1, &instvbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vvbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float)*3));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, instvbo);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CelestialBody), (void*)0);
        glVertexAttribDivisor(2, 1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CelestialBody), (void*)sizeof(glm::vec3));
        glVertexAttribDivisor(3, 1);
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(CelestialBody), (void*)(sizeof(glm::vec3)*2));
        glVertexAttribDivisor(4, 1);
        glEnableVertexAttribArray(4);

        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(CelestialBody), (void*)(sizeof(glm::vec3)*2 + sizeof(float)));
        glVertexAttribDivisor(5, 1);
        glEnableVertexAttribArray(5);
    }

    glBindBuffer(GL_ARRAY_BUFFER, instvbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * (sizeof(CelestialBody)), positions.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(vao);

    static GLint camPosLoc = glGetUniformLocation(shader.shaderID, "camPos");
    static GLint mvpLoc = glGetUniformLocation(shader.shaderID, "mvp");
    static GLint timeLoc = glGetUniformLocation(shader.shaderID, "time");
    static GLint rotLoc = glGetUniformLocation(shader.shaderID, "rot");
    static GLint texLoc = glGetUniformLocation(shader.shaderID, "ourTexture");

    glUniform1i(texLoc, 4);
    glUniform3f(camPosLoc, camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(camera->mvp));
    glUniform1f(timeLoc, static_cast<float>(glfwGetTime()));
    glUniform1f(rotLoc, (std::fmod((timeOfDay - (dayLength/2.0f)), dayLength) / dayLength) * (2.0f * 3.1415926535897932384626433832795028841971693993751f));

    sunAndMoonTex.bind_to_unit(4);

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 8, positions.size());

    glBindVertexArray(0);
    glUseProgram(0);
}
