//
// Created by jack on 3/5/2025.
//

#ifndef FULLSCREENKALEIDOSCOPE_H
#define FULLSCREENKALEIDOSCOPE_H

#include "../PrecompHeader.h"
#include "../Shader.h"

inline void drawFullscreenKaleidoscope()
{
    static jl::Shader shader(
        R"glsl(
            #version 330 core

            layout (location = 0) in vec2 aPos;     // NDC position
            layout (location = 1) in vec2 aTexCoord; // Texture coordinates

            out vec2 FragPos;

            void main()
            {
                gl_Position = vec4(aPos, 0.0, 1.0); // Already in NDC
                FragPos = aPos; // Pass position to fragment shader
            }
        )glsl",
        R"glsl(
            #version 330 core

            in vec2 FragPos;
            out vec4 FragColor;

            uniform float uTime;

            // Signed distance function for star shape
            float sdStar(vec2 p, float r, int n, float m) {
                float an = 3.141593/float(n);
                float en = 3.141593/m;
                vec2 acs = vec2(cos(an),sin(an));
                vec2 ecs = vec2(cos(en),sin(en));

                float bn = mod(atan(p.x,p.y),2.0*an) - an;
                p = length(p)*vec2(cos(bn),abs(sin(bn)));
                p -= r*acs;
                p += ecs*clamp(-dot(p,ecs), 0.0, r*acs.y/ecs.y);

                return length(p)*sign(p.x);
            }

            void main()
            {
                // Center and scale
                vec2 uv = FragPos.xy; // Already in -1 to 1 range

                // Calculate distance from center
                float dist = length(uv);

                // Angle from center
                float angle = atan(uv.y, uv.x);

                // Create radiating stripes
                float stripeFreq = 20.0 + 10.0 * sin(uTime * 0.2); // Number of stripes
                float stripeWidth = 0.5 + 0.3 * sin(uTime * 0.3); // Width variation
                float radialStripes = sin(angle * stripeFreq + uTime * 2.0);
                radialStripes = step(stripeWidth, radialStripes); // Make it binary for B&W

                // Create concentric rings
                float ringFreq = 15.0 + 5.0 * sin(uTime * 0.1); // Frequency of rings
                float rings = sin(dist * ringFreq - uTime * 3.0);
                rings = step(0.0, rings); // Make it binary for B&W

                // Combine patterns with XOR effect for more hypnotic appearance
                float pattern = mod(radialStripes + rings, 2.0);

                // Add subtle star burst effect
                float starTime = uTime * 0.5;
                float starScale = 0.2 + 0.1 * sin(uTime * 0.4);
                float star = sdStar(uv, starScale, 8, 0.5);
                star = 1.0 - smoothstep(0.0, 0.1, abs(star));

                // Pulsing star in the center
                float centerPulse = 0.1 + 0.05 * sin(uTime * 4.0);
                float centerStar = 1.0 - smoothstep(0.0, centerPulse, dist);

                // Combine everything
                float finalPattern = mix(pattern, 1.0, star * 0.5);
                finalPattern = mix(finalPattern, 1.0, centerStar);

                // Create black and white output
                vec3 color = vec3(finalPattern);

                // Final color
                FragColor = vec4(color, 1.0);
            }
        )glsl",
        "hypnosisShader"
    );
    glUseProgram(shader.shaderID);

    // Simple full-screen quad with NDC coordinates
    static const float vertices[] = {
        // x     y     u     v
        -1.0f, -1.0f, 0.0f, 0.0f,  // bottom left
         1.0f, -1.0f, 1.0f, 0.0f,  // bottom right
        -1.0f,  1.0f, 0.0f, 1.0f,  // top left

        -1.0f,  1.0f, 0.0f, 1.0f,  // top left
         1.0f, -1.0f, 1.0f, 0.0f,  // bottom right
         1.0f,  1.0f, 1.0f, 1.0f   // top right
    };

    static GLuint vao = 0;
    static GLuint vbo = 0;
    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float)*2));
    }

    glBindVertexArray(vao);

    // Update time uniform
    static GLint timeUni = glGetUniformLocation(shader.shaderID, "uTime");
    glUniform1f(timeUni, glfwGetTime());

    // Draw the fullscreen quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#endif //FULLSCREENKALEIDOSCOPE_H
