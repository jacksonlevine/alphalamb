//
// Created by jack on 3/29/2025.
//

#ifndef COMPUTERSCREENRENDERER_H
#define COMPUTERSCREENRENDERER_H
#include "Camera.h"
#include "Shader.h"
#include "TextEditor.h"

void drawTextEditor(TextEditor& editor);


class ComputerScreenRenderer {
public:
    GLuint fbo = 0, scrnTex = 0;
    int cwidth = 800, cheight = 600;
    ComputerScreenRenderer()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glGenTextures(1, &scrnTex);
        glBindTexture(GL_TEXTURE_2D, scrnTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cwidth, cheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //attach screen texture to fbo:
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scrnTex, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void renderEditorToFBO(TextEditor& editor) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, cwidth, cheight);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::NewFrame();
        drawTextEditor(editor);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    ComputerScreenRenderer& operator=(const ComputerScreenRenderer& other) = delete;
    ComputerScreenRenderer(ComputerScreenRenderer& other) = delete;
    ComputerScreenRenderer& operator=(ComputerScreenRenderer&& other) noexcept {
        if(this != &other) {
            fbo = other.fbo;
            scrnTex = other.scrnTex;
        }
        return *this;
    }
    ComputerScreenRenderer(ComputerScreenRenderer&& other) noexcept {
        fbo = other.fbo;
        scrnTex = other.scrnTex;
    }
    template<class Archive>
    void serialize(Archive& archive) {}
    ~ComputerScreenRenderer()
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &scrnTex);
    }

    void drawAtPos(const glm::vec3& pos, jl::Camera& camera)
    {
        using namespace glm;



        auto shad = jl::Shader(R"glsl(
                #version 330 core
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoord;
            void main()
            {
            }

            )glsl",
            R"glsl(

            )glsl",
            "compScreenShad");
        
        static std::vector<vec3> vertices = {
            vec3(0.f, 0.f, 0.f),
            vec3(0.f, 1.f, 0.f),
            vec3(1.f, 1.f, 0.f),
            
            vec3(1.f, 1.f, 0.f),
            vec3(1.f, 0.f, 0.f),
            vec3(0.f, 0.f, 0.f),
        };
        static std::vector<vec2> texs = {
            vec2(0.f, 0.f),
            vec2(0.f, 1.f),
            vec2(1.f, 1.f),

            vec2(1.f, 1.f),
            vec2(1.f, 0.f),
            vec2(0.f, 0.f),
        };

        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint uvvbo = 0;

        if(vao == 0)
        {
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &uvvbo);

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

            glBindBuffer(GL_ARRAY_BUFFER, uvvbo);
            glBufferData(GL_ARRAY_BUFFER, texs.size() * sizeof(vec2), texs.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
        }


    }
};



#endif //COMPUTERSCREENRENDERER_H
