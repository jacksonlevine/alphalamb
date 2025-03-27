//
// Created by jack on 3/26/2025.
//

#include "ComputerComponent.h"


void drawTextEditor(TextEditor& editor)
{
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
}
