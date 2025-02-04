//
// Created by jack on 2/3/2025.
//

#include "ParticlesGizmo.h"

#include "../../Shader.h"

void ParticlesGizmo::draw(World* world, Player* player) const
{

}

void ParticlesGizmo::init()
{
    jl::Shader shader(
        R"glsl(
            #version 330 core
            layout(location = 0) in vec3 vertexPosition; // Quad vertex positions
            layout(location = 1) in float cornerID;    // Corner ID of quad
            layout(location = 2) in float blockID;

            layout(location = 3) in vec4 instancePosition;

            uniform mat4 mvp;

            void main() {

            }

        )glsl",
        R"glsl(

        )glsl",
        "particleShader"
        );
}
