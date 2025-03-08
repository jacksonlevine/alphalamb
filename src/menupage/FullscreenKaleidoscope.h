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
uniform vec2 uResolution;

#define TIME        uTime
#define RESOLUTION  uResolution
#define PI          3.141592654
#define TAU         (2.0*PI)
#define LESS(a,b,c) mix(a,b,step(0.,c))
#define SABS(x,k)   LESS((.5/(k))*(x)*(x)+(k)*.5,abs(x),abs(x)-(k))
#define ROT(x)      mat2(cos(x), -sin(x), sin(x), cos(x))
#define PSIN(x)     (0.5+0.5*sin(x))

// Array of rotation matrices
const mat2 rotations[4] = mat2[](
    ROT(0.0*PI/2.0), ROT(1.0*PI/2.0), ROT(2.0*PI/2.0), ROT(3.0*PI/2.0)
);

float rand(vec2 n) {
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec2 toPolar(vec2 p) {
    return vec2(length(p), atan(p.y, p.x));
}

vec2 toRect(vec2 p) {
    return p.x * vec2(cos(p.y), sin(p.y));
}

vec2 mod2_1(inout vec2 p) {
    vec2 pp = p + 0.5;
    vec2 nn = floor(pp);
    p = fract(pp) - 0.5;
    return nn;
}

float modMirror1(inout float p, float size) {
    float halfsize = size * 0.5;
    float c = floor((p + halfsize) / size);
    p = mod(p + halfsize, size) - halfsize;
    p *= mod(c, 2.0) * 2.0 - 1.0;
    return c;
}

float circle(vec2 p, float r) {
    return length(p) - r;
}

float smoothKaleidoscope(inout vec2 p, float sm, float rep) {
    vec2 hp = p;
    vec2 hpp = toPolar(hp);
    float rn = modMirror1(hpp.y, TAU / rep);
    float sa = PI / rep - SABS(PI / rep - abs(hpp.y), sm);
    hpp.y = sign(hpp.y) * (sa);
    hp = toRect(hpp);
    p = hp;
    return rn;
}

// Star field generation
float stars(vec2 p, float density) {
    vec2 pp = floor(p * 500.0);
    float r = rand(pp);
    if (r > 1.0 - density) {
        float sparkle = pow(0.5 + 0.5 * sin(TIME * (r * 5.0) + r * 10.0), 15.0);
        return (r - (1.0 - density)) / density * sparkle * 0.8;
    }
    return 0.0;
}

// Nebula-like function
vec3 nebula(vec2 p, float time) {
    vec2 uv = p * 0.2;
    float t = time * 0.1;

    // Create base noise layers
    float n1 = sin(uv.x * 1.5 + t) * sin(uv.y * 1.0 - t) * 0.5 + 0.5;
    float n2 = sin(uv.x * 2.3 - t) * sin(uv.y * 1.8 + t * 1.5) * 0.5 + 0.5;
    float n3 = sin(uv.x * 3.7 + t * 0.5) * sin(uv.y * 2.9 - t * 0.7) * 0.5 + 0.5;

    // Combine noise layers
    float finalNoise = pow(n1 * n2 * n3, 2.0);

    // Color mapping
    vec3 col1 = vec3(0.05, 0.0, 0.15); // Deep space blue
    vec3 col2 = vec3(0.3, 0.0, 0.5);   // Purple
    vec3 col3 = vec3(0.0, 0.2, 0.4);   // Blue
    vec3 col4 = vec3(0.1, 0.4, 0.6);   // Light blue

    // Create color gradient based on noise
    vec3 color = mix(col1, col2, smoothstep(0.0, 0.3, finalNoise));
    color = mix(color, col3, smoothstep(0.3, 0.6, finalNoise));
    color = mix(color, col4, smoothstep(0.6, 0.9, finalNoise));

    // Add some dust-like turbulence
    float dust = pow(rand(p * 10.0 + time), 10.0) * 0.2;
    color += dust * vec3(0.4, 0.4, 0.6);

    return color * smoothstep(0.1, 0.3, finalNoise);
}

float cell0(vec2 p) {
    float d0 = circle(p + 0.5, 0.5);
    float d1 = circle(p - 0.5, 0.5);
    float d = min(d0, d1);
    return d;
}

float cell1(vec2 p) {
    float d0 = abs(p.x);
    float d1 = abs(p.y);
    float d2 = circle(p, 0.25);
    float d = min(min(d0, d1), d2);
    return d;
}

float cell(vec2 p, vec2 cp, vec2 n, float lw) {
    float r = rand(n + 1237.0);
    cp *= rotations[int(4.0 * r)];
    float rr = fract(13.0 * r);
    float d = (rr > 0.25) ? cell0(cp) : cell1(cp);
    return abs(d) - lw;
}

float truchet(vec2 p, float lw) {
    float s = 0.1;
    p /= s;
    vec2 cp = p;
    vec2 n = mod2_1(cp);
    float d = cell(p, cp, n, lw) * s;
    return d;
}

float df(vec2 p, float rep, float time) {
    const float lw = 0.05;
    vec2 pp = toPolar(p);
    pp.x /= 1.0 + pp.x;
    p = toRect(pp);
    vec2 cp = p;
    float sm = 3.0 / rep;
    float n = smoothKaleidoscope(cp, sm, rep);
    cp *= ROT(0.05 * time);
    cp -= 0.1 * time;
    return truchet(cp, lw * mix(0.25, 2.0, PSIN(-2.0 * time + 5.0 * cp.x + (0.5 * rep) * cp.y)));
}

vec3 spaceColor(vec2 q, vec2 p, float rep, float tm) {
    vec2 pp = toPolar(p);
    float d = df(p, rep, tm);

    // Base deep space color
    vec3 col = vec3(0.0, 0.0, 0.05);

    // Add nebula effect
    col += nebula(p, tm);

    // Add star field
    col += vec3(stars(p, 0.4)) * vec3(0.8, 0.9, 1.0);

    // Add structure from the original kaleidoscope
    float aa = 2.0 / RESOLUTION.y;
    vec3 kaleColor = mix(
        vec3(0.0, 0.1, 0.2),
        vec3(0.1, 0.3, 0.6),
        smoothstep(-aa, aa, -d)
    );

    // Add shimmering effect to the structure
    kaleColor *= 0.5 + 0.5 * pow(sin(tm * 2.0 + pp.x * 3.0), 2.0);

    // Blend nebula and kaleidoscope
    col = mix(col, kaleColor, 0.7 * smoothstep(-aa, aa, -d));

    return col;
}

vec3 postProcess(vec3 col, vec2 q) {
    // Add subtle vignette
    col *= 0.5 + 0.5 * pow(16.0 * q.x * q.y * (1.0 - q.x) * (1.0 - q.y), 0.3);

    // Enhance contrast
    col = pow(col, vec3(1.2));

    // Color grading - pull slightly toward deep blue
    col = mix(col, vec3(0.1, 0.1, 0.3), 0.1);

    return col;
}

void main() {
    vec2 q = FragPos * 0.5 + 0.5; // Normalize to [0, 1]
    vec2 p = -1.0 + 2.0 * q;
    p.x *= RESOLUTION.x / RESOLUTION.y;
    vec2 op = p;
    vec3 col = vec3(0.0);
    float tm = TIME * 0.15; // Slower movement for space feel

    // Add distant stars first (background layer)
    col = vec3(0.01, 0.01, 0.03); // Deep space background
    col += vec3(stars(p * 0.5, 0.7)) * vec3(0.7, 0.8, 1.0);

    // Layer multiple kaleidoscopic structures
    float aa = -1.0 + 0.5 * length(p);
    float a = 0.8; // Reduced intensity for more space-like feel
    float ra = tanh(length(0.5 * p));

    // Fewer layers for cleaner space look
    for (int i = 0; i < 4; ++i) {
        p *= ROT(sqrt(0.1 * float(i)) * tm - ra);
        col += a * spaceColor(q, p, 20.0 - 4.0 * float(i), 0.8 * tm + float(i));
        a *= aa * 0.9;
    }

    // Add final distant nebula background
    col += nebula(op * 0.2, tm) * 0.3;

    // Post-processing
    col = postProcess(col, q);

    // Enhance brightness of highlights for star-like shimmer
    col = mix(col, vec3(1.0), pow(length(col), 5.0) * 0.5);

    FragColor = vec4(col, 1.0);
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

    // Update resolution uniform
    static GLint resUni = glGetUniformLocation(shader.shaderID, "uResolution");
    glUniform2f(resUni, 1280.0f, 1024.0f); // Replace with actual screen resolution

    // Draw the fullscreen quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#endif //FULLSCREENKALEIDOSCOPE_H
