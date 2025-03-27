//
// Created by jack on 3/26/2025.
//

#ifndef COMPUTERCOMPONENT_H
#define COMPUTERCOMPONENT_H
#include "../TextEditor.h"

void drawTextEditor(TextEditor& editor);

class ComputerComponent {
public:
    TextEditor editor = {};
    GLuint fbo = 0, scrnTex = 0;
    constexpr int cwidth = 800, cheight = 600;
    ComputerComponent() {
        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Python());
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
    void renderScreenToFBO() {
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
    ComputerComponent& operator=(const ComputerComponent& other) = delete;
    ComputerComponent(ComputerComponent& other) = delete;
    ComputerComponent& operator=(ComputerComponent&& other) noexcept {
        if(this != &other) {
            fbo = other.fbo;
            scrnTex = other.scrnTex;
            editor = other.editor;
        }
        return *this;
    }
    ComputerComponent(ComputerComponent&& other) noexcept {
        fbo = other.fbo;
        scrnTex = other.scrnTex;
        editor = other.editor;
    }
    template<class Archive>
    void serialize(Archive& archive) {}
	~ComputerComponent()
    {
    	glDeleteFramebuffers(1, &fbo);
    	glDeleteTextures(1, &scrnTex);
    }
};



#endif //COMPUTERCOMPONENT_H
