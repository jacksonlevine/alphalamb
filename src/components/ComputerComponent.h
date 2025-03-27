//
// Created by jack on 3/26/2025.
//

#ifndef COMPUTERCOMPONENT_H
#define COMPUTERCOMPONENT_H
#include "../TextEditor.h"


class ComputerComponent {
public:
    TextEditor editor = {};
    GLuint fbo = 0, texture = 0;
    constexpr int cwidth = 800, cheight = 600;
    ComputerComponent() {
        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Python());
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cwidth, cheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //attach texture to fbo:
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void renderToFBO() {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, cwidth, cheight);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::NewFrame();
        auto cpos = editor.GetCursorPosition();
        ImGui::Begin("Computer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_Always);
        ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        if (ImGui::BeginMenuBar()) {
	        if (ImGui::BeginMenu("File")) {
		        if (ImGui::MenuItem("Quit", "Alt-F4"))
		        ImGui::EndMenu();
	        }
	        if (ImGui::BeginMenu("Edit")) {
		        bool ro = editor.IsReadOnly();
		        if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
			        editor.SetReadOnly(ro);
		        ImGui::Separator();
		        if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
			        editor.Undo();
		        if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
			        editor.Redo();
		        ImGui::Separator();
		        if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
			        editor.Copy();
		        if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
			        editor.Cut();
		        if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
			        editor.Delete();
		        if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
			        editor.Paste();
		        ImGui::Separator();
		        if (ImGui::MenuItem("Select all", nullptr, nullptr))
			        editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));
		        ImGui::EndMenu();
	        }
	        if (ImGui::BeginMenu("View")) {
		        if (ImGui::MenuItem("Dark palette"))
			        editor.SetPalette(TextEditor::GetDarkPalette());
		        if (ImGui::MenuItem("Light palette"))
			        editor.SetPalette(TextEditor::GetLightPalette());
		        if (ImGui::MenuItem("Retro blue palette"))
			        editor.SetPalette(TextEditor::GetRetroBluePalette());
		        ImGui::EndMenu();
	        }
	        ImGui::EndMenuBar();
        }
        ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
	        editor.IsOverwrite() ? "Ovr" : "Ins",
	        editor.CanUndo() ? "*" : " ",
	        editor.GetLanguageDefinition().mName.c_str(), "test.py");
        editor.Render("Code Edit");
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    ComputerComponent& operator=(const ComputerComponent& other) = delete;
    ComputerComponent(ComputerComponent& other) = delete;
    ComputerComponent& operator=(ComputerComponent&& other) noexcept {
        if(this != &other) {
            fbo = other.fbo;
            texture = other.texture;
            editor = other.editor;
        }
        return *this;
    }
    ComputerComponent(ComputerComponent&& other) noexcept {
        fbo = other.fbo;
        texture = other.texture;
        editor = other.editor;
    }
    template<class Archive>
    void serialize(Archive& archive) {}
	~ComputerComponent()
    {
    	glDeleteFramebuffers(1, &fbo);
    	glDeleteTextures(1, &texture);
    }
};



#endif //COMPUTERCOMPONENT_H
