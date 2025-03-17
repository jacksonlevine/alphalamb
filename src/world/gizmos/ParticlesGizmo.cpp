//
// Created by jack on 2/3/2025.
//

#include "ParticlesGizmo.h"

#include "../../LUTLoader.h"
#include "../../Shader.h"

void ParticlesGizmo::draw(World* world, entt::entity playerIndex, entt::registry& reg)
{
    updateParticlesCollisionCage(world);
    glBindVertexArray(vao);
    glUseProgram(shaderProgram);
    static GLuint mvploc = glGetUniformLocation(shaderProgram, "mvp");
    static GLuint camPosloc = glGetUniformLocation(shaderProgram, "camPos");

    auto camera = reg.get<jl::Camera>(playerIndex);

    glUniformMatrix4fv(mvploc, 1, GL_FALSE, glm::value_ptr(camera.mvp));
    glUniform3f(camPosloc, camera.transform.position.x, camera.transform.position.y, camera.transform.position.z);

    glUniform1i(glGetUniformLocation(shaderProgram, "lut"), 1);

    updateParticlesToPhysicsSpots();
    sendUpdatedInstancesList();

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instances.size());
}

void ParticlesGizmo::init()
{
    jl::Shader shader(
        R"glsl(
            #version 330 core
//Main
            layout(location = 0) in vec3 vertexPosition;
            layout(location = 1) in float cornerID;

//Instanced
            layout(location = 2) in float blockID;
            layout(location = 3) in vec4 instancePosition;

            uniform mat4 mvp;
            uniform vec3 camPos;
            uniform vec2 blockSideTextures[93];

            out vec2 TexCoord;

            void main() {

                vec3 realPosition = instancePosition.xyz;


                vec3 look = normalize(realPosition - camPos);
                vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), look));
                vec3 up = normalize(cross(look, right));

                vec3 billboardedPosition = realPosition + ((vertexPosition.x * instancePosition.a) * right + (vertexPosition.y  * instancePosition.a) * up);



            const vec2 texOffsets[4] = vec2[4](
                vec2(0.0,0.0),
                vec2(0.0078125,0.0),
                vec2(0.0078125, -0.0078125),
                vec2(0.0, -0.0078125)

            );




vec2 texSpot = blockSideTextures[int(blockID)];
vec2 baseUV = vec2(texSpot.x * 0.03308823529411764705882352941176f, 1.0f - ((texSpot.y/16.0f) * 0.52941176470588235294117647058824f));

                vec2 realUV = baseUV + texOffsets[int(cornerID)];

                TexCoord = realUV;
                gl_Position = mvp * vec4(billboardedPosition, 1.0);
            }

        )glsl",
        R"glsl(
            #version 330 core
            in vec2 TexCoord;
            out vec4 FragColor;
            uniform sampler2D ourTexture;
            uniform sampler3D lut;

            void main()
            {
                vec4 texColor = texture(ourTexture, TexCoord);
                FragColor = texColor;
                if(texColor.a < 0.1) {
                    discard;
                }

                vec3 lutCoords = clamp(FragColor.xyz, 0.0, 1.0);
                lutCoords = lutCoords * (63.0/64.0) + (0.5/64.0);
                vec4 lutTex = texture(lut, lutCoords);

                FragColor = vec4(lutTex.xyz, FragColor.a);
            }
        )glsl",
        "particleShader"
        );
    shaderProgram = shader.shaderID;
    sendBlockSideTexturesToShader(shaderProgram);



    float mainModel[] = {
        //position         //corner id
        -0.5, -0.5, 0.0,    0,
        -0.5, 0.5, 0.0,     3,
        0.5, 0.5, 0.0,      2,

        0.5, 0.5, 0.0,      2,
        0.5, -0.5, 0.0,     1,
        -0.5, -0.5, 0.0,    0
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &mainvbo);
    glBindBuffer(GL_ARRAY_BUFFER, mainvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mainModel), mainModel, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &instancesvbo);




}
