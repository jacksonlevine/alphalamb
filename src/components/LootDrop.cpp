//
// Created by jack on 5/15/2025.
//

#include "LootDrop.h"

#include "NPPositionComponent.h"
#include "../ModelLoader.h"
#include "../Shader.h"
#include "../Scene.h"

void renderLootDrops(entt::registry& reg, Scene* scene)
{

    struct LootDisplayInstance
    {
        glm::vec3 pos;
    };

    static std::vector<LootDisplayInstance> lootDisplayInstances;


    static jl::Shader shader(
        R"glsl(
            #version 330 core
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoord;

            layout(location = 2) in vec3 instancePos;

            out vec2 TexCoord;

            uniform mat4 mvp;
            out vec3 vertPos;
            void main()
            {
                // Translate the rotated position by `pos`
                vec4 worldPosition = vec4(inPosition + instancePos, 1.0);

                // Transform to clip space using the MVP matrix
                gl_Position = mvp * worldPosition;

                // Pass texture coordinates to the fragment shader
                TexCoord = inTexCoord;
                vertPos = worldPosition.xyz;
            }
        )glsl",
        R"glsl(
            #version 330 core
            out vec4 FragColor;

            uniform float hideClose;

            in vec2 TexCoord;
            in vec3 vertPos;

            uniform sampler2D texture1;
            uniform vec3 camPos;

            void main()
            {
                FragColor = texture(texture1, TexCoord);
                if (hideClose > 0.4) {
                float radius = 200.0;
                float fadeStart = 190.0;  // Start fading at 40 units
                float dist = distance(vertPos, camPos);

                // Ensure alpha is 1.0 if beyond radius, and smoothly fades inside
                float alphaFactor = clamp((dist - fadeStart) / (radius - fadeStart), 0.0, 1.0);
                FragColor.a *= alphaFactor;
                if(FragColor.a < 0.1) {
                    discard;
                }
    }
            }
        )glsl",
        "lootDropGltfInstanceSHader"

    );



    static jl::ModelAndTextures basemodeltex = jl::ModelLoader::loadModel("resources/models/drop1.glb", false);


    glBindVertexArray(basemodeltex.modelGLObjects.at(0).vao);

    glUseProgram(shader.shaderID);

    //glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
    //glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, scene->worldtex);


    glUniform1i(glGetUniformLocation(shader.shaderID, "texture1"), 0);

    auto & camera = scene->our<jl::Camera>();

    glUniform1f(glGetUniformLocation(shader.shaderID, "hideClose"), 0.0f);
    glUniform3f(glGetUniformLocation(shader.shaderID, "camPos"),0.f, 0.f, 0.f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));

    auto view = reg.view<LootDrop, NPPositionComponent>();
    lootDisplayInstances.clear();
    for (auto entity : view)
    {
        //std::cout << "entity: "  << (int)entity << std::endl;
        auto [pos] = view.get<NPPositionComponent>(entity);
        lootDisplayInstances.emplace_back(pos);
    }

    static GLuint instancevbo = 0;

    auto  &mglo = basemodeltex.modelGLObjects.at(0);

        glBindVertexArray(mglo.vao);

        if (instancevbo == 0 )
        {
            glGenBuffers(1, &instancevbo);

        }

            glBindBuffer(GL_ARRAY_BUFFER, instancevbo);

            glBufferData(GL_ARRAY_BUFFER, sizeof(LootDisplayInstance) * lootDisplayInstances.size(), lootDisplayInstances.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(LootDisplayInstance), (GLvoid*)0);
            glEnableVertexAttribArray(2);
            glVertexAttribDivisor(2, 1);


        glDrawElementsInstanced(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr, lootDisplayInstances.size());

}

