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

vec3 color(vec2 q, vec2 p, float rep, float tm) {
    vec2 pp = toPolar(p);
    float d = df(p, rep, tm);
    vec3 col = vec3(0.0);
    float aa = 2.0 / RESOLUTION.y;
    vec3 baseCol = hsv2rgb(vec3(-0.25 * tm + sin(5.5 * d), tanh(pp.x), 1.0));
    col = mix(col, baseCol, smoothstep(-aa, aa, -d));
    col = baseCol + 0.5 * col.zxy;
    return col;
}

vec3 postProcess(vec3 col, vec2 q) {
    col = pow(clamp(col, 0.0, 1.0), vec3(0.75));
    col = col * 0.6 + 0.4 * col * col * (3.0 - 2.0 * col);
    col = mix(col, vec3(dot(col, vec3(0.33))), -0.4);
    col *= 0.5 + 0.5 * pow(19.0 * q.x * q.y * (1.0 - q.x) * (1.0 - q.y), 0.7);
    return col;
}

void main() {
    vec2 q = FragPos * 0.5 + 0.5; // Normalize to [0, 1]
    vec2 p = -1.0 + 2.0 * q;
    p.x *= RESOLUTION.x / RESOLUTION.y;
    vec2 op = p;
    vec3 col = vec3(0.0);
    float tm = TIME * 0.25;
    float aa = -1.0 + 0.5 * length(p);
    float a = 1.0;
    float ra = tanh(length(0.5 * p));

    for (int i = 0; i < 6; ++i) {
        p *= ROT(sqrt(0.1 * float(i)) * tm - ra);
        col += a * color(q, p, 30.0 - 6.0 * float(i), 1.0 * tm + float(i));
        a *= aa;
    }

    col = tanh(col);
    col = abs(p.y - col);
    col = max(1.0 - col, 0.0);
    col = pow(col, tanh(0.0 + length(op) * 1.0 * vec3(1.0, 1.5, 3.0)));
    col = postProcess(col, q);

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
