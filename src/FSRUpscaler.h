//
// Created by jack on 2/9/2025.
//

#ifndef FSRUPSCALER_H
#define FSRUPSCALER_H

#include "PrecompHeader.h"
#include "Shader.h"

class FSR {
private:
    GLuint easu_program;
    GLuint rcas_program;
    GLuint input_texture;
    GLuint intermediate_fbo;
    GLuint intermediate_texture;
    GLuint quad_vao;

    int input_width, input_height;
    int output_width, output_height;
    float sharpness;


    // FSR RCAS shader (Robust Contrast Adaptive Sharpening)
    const char* rcas_compute = R"(
        #version 450
        layout(local_size_x = 64) in;

        layout(binding = 0) uniform sampler2D InputTexture;
        layout(binding = 1, rgba16f) uniform image2D OutputTexture;

        layout(std140, binding = 2) uniform RCASUniforms {
            vec4 Const0;
        };

        #define A_GPU 1
        #define A_GLSL 1

        // Include the actual FSR RCAS implementation
        #include "ffx_fsr1.h"

        void main() {
            uvec2 WorkGroupId = gl_WorkGroupID.xy;
            uvec2 LocalThreadId = gl_LocalInvocationID.xy;
            FsrRcas(OutputTexture, WorkGroupId, LocalThreadId, InputTexture, Const0);
        }
    )";

public:
    FSR(int in_width, int in_height, int out_width, int out_height, float sharp = 0.2f)
        : input_width(in_width), input_height(in_height)
        , output_width(out_width), output_height(out_height)
        , sharpness(sharp) {

        std::ifstream file("resources/shad.glsl");  // Open the file
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << std::endl;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();  // Read the entire file into the buffer

        // Create compute shader programs
        easu_program = createComputeProgram(buffer.str().c_str());
        //rcas_program = createComputeProgram(rcas_compute);

        // Create intermediate texture for EASU output
        glCreateTextures(GL_TEXTURE_2D, 1, &intermediate_texture);
        glTextureStorage2D(intermediate_texture, 1, GL_RGBA16F, output_width, output_height);

        // Calculate EASU constants
        // These values come from AMD's FSR documentation
        float inputViewportInPixelsX = static_cast<float>(input_width);
        float inputViewportInPixelsY = static_cast<float>(input_height);
        float outputViewportInPixelsX = static_cast<float>(output_width);
        float outputViewportInPixelsY = static_cast<float>(output_height);

        using namespace glm;
        vec4 const0, const1, const2, const3;

        // Set up constants for EASU
        const0.x = inputViewportInPixelsX / outputViewportInPixelsX;
        const0.y = inputViewportInPixelsY / outputViewportInPixelsY;
        const0.z = 0.5f * inputViewportInPixelsX / outputViewportInPixelsX - 0.5f;
        const0.w = 0.5f * inputViewportInPixelsY / outputViewportInPixelsY - 0.5f;

        const1.x = 1.0f / inputViewportInPixelsX;
        const1.y = 1.0f / inputViewportInPixelsY;
        const1.z = 1.0f / outputViewportInPixelsX;
        const1.w = 1.0f / outputViewportInPixelsY;

        // Upload constants to EASU shader
        GLuint easu_ubo;
        glCreateBuffers(1, &easu_ubo);
        glNamedBufferStorage(easu_ubo, sizeof(vec4) * 4, nullptr, GL_DYNAMIC_STORAGE_BIT);
        glNamedBufferSubData(easu_ubo, 0, sizeof(vec4), &const0);
        glNamedBufferSubData(easu_ubo, sizeof(vec4), sizeof(vec4), &const1);
        glNamedBufferSubData(easu_ubo, sizeof(vec4) * 2, sizeof(vec4), &const2);
        glNamedBufferSubData(easu_ubo, sizeof(vec4) * 3, sizeof(vec4), &const3);

        // // Calculate RCAS constant (sharpness)
        // vec4 rcas_const;
        // rcas_const.x = sharpness;
        //
        // // Upload constant to RCAS shader
        // GLuint rcas_ubo;
        // glCreateBuffers(1, &rcas_ubo);
        // glNamedBufferStorage(rcas_ubo, sizeof(vec4), &rcas_const, GL_DYNAMIC_STORAGE_BIT);

        // Bind uniform buffers
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, easu_ubo);
        //glBindBufferBase(GL_UNIFORM_BUFFER, 2, rcas_ubo);
    }

    void upscale(GLuint input_tex, GLuint output_tex) {
        // EASU Pass (upscaling)
        glUseProgram(easu_program);

        glBindImageTexture(1, intermediate_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        glBindTextureUnit(0, input_tex);

        glDispatchCompute((output_width + 15) / 16, (output_height + 15) / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // // RCAS Pass (sharpening)
        // glUseProgram(rcas_program);
        //
        // glBindImageTexture(1, output_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        // glBindTextureUnit(0, intermediate_texture);
        //
        // glDispatchCompute((output_width + 15) / 16, (output_height + 15) / 16, 1);
        // glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

private:
    GLuint createComputeProgram(const char* computeShaderSource) {
        GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(computeShader, 1, &computeShaderSource, nullptr);
        glCompileShader(computeShader);

        GLint success;
        glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(computeShader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation error: " << infoLog << std::endl;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, computeShader);
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Shader linking error: " << infoLog << std::endl;
        }

        glDeleteShader(computeShader);
        return program;
    }
};


class RenderPipeline {
private:
    GLuint scene_fbo;
    GLuint scene_texture;
    GLuint scene_depth;
    GLuint output_texture;
    GLuint quad_vao;
    GLuint quad_vbo;
    GLuint display_shader;

    int input_width, input_height;
    int output_width, output_height;

    bool checkFramebufferStatus(GLuint fbo) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer incomplete! Status: " << status << std::endl;
            return false;
        }
        return true;
    }

public:
    RenderPipeline(int in_width, int in_height, int out_width, int out_height)
        : input_width(in_width), input_height(in_height)
        , output_width(out_width), output_height(out_height) {

        // Create scene framebuffer
        glGenFramebuffers(1, &scene_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);

        // Create scene color texture
        glGenTextures(1, &scene_texture);
        glBindTexture(GL_TEXTURE_2D, scene_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, input_width, input_height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Create scene depth texture
        glGenTextures(1, &scene_depth);
        glBindTexture(GL_TEXTURE_2D, scene_depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, input_width, input_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Attach textures to scene FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_texture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, scene_depth, 0);

        if (!checkFramebufferStatus(scene_fbo)) {
            throw std::runtime_error("Scene framebuffer incomplete!");
        }

        // Create output texture
        glGenTextures(1, &output_texture);
        glBindTexture(GL_TEXTURE_2D, output_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, output_width, output_height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Create quad for display
        float quad_vertices[] = {
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &quad_vao);
        glGenBuffers(1, &quad_vbo);
        glBindVertexArray(quad_vao);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        // Create display shader
        const char* quad_vert = R"(
            #version 450
            layout(location = 0) in vec2 aPos;
            layout(location = 1) in vec2 aTexCoord;
            out vec2 TexCoord;
            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        const char* quad_frag = R"(
            #version 450
            in vec2 TexCoord;
            out vec4 FragColor;
            uniform sampler2D screenTexture;
            void main() {
                vec4 color = texture(screenTexture, TexCoord);
                FragColor = color;
            }
        )";

        display_shader = jl::Shader(quad_vert, quad_frag, "quadshader").shaderID;
    }

    void beginSceneRender() {
        glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
        glViewport(0, 0, input_width, input_height);

        // Debug check
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer not complete in beginSceneRender!" << std::endl;
        }
    }

    void endSceneRender() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void displayResult() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, output_width, output_height);

        glUseProgram(display_shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, output_texture);
        glUniform1i(glGetUniformLocation(display_shader, "screenTexture"), 0);

        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Debug - check for GL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "OpenGL error in displayResult: " << err << std::endl;
        }
    }

    GLuint getSceneTexture() const { return scene_texture; }
    GLuint getOutputTexture() const { return output_texture; }
};
#endif //FSRUPSCALER_H
