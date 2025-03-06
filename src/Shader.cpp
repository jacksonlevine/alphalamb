//
// Created by jack on 10/13/2024.
//

#include "Shader.h"

namespace jl {

Shader::Shader(const char *vert, const char *frag, const char *name) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, &vert, nullptr);
    glCompileShader(vertexShader);
    glShaderSource(fragmentShader, 1, &frag, nullptr);
    glCompileShader(fragmentShader);
    GLint success;
    char errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, errorLog);
        std::cerr << name << " vert shade comp error:" << errorLog << '\n';
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, errorLog);
        std::cerr << name << " frag shade comp error:" << errorLog << '\n';
    }
    shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);
    glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, errorLog);
        std::cerr << name << " shader program link error:" << errorLog << '\n';
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

}