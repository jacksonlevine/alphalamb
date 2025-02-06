//
// Created by jack on 11/25/2024.
//

#ifndef IMGUISTUFF_H
#define IMGUISTUFF_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


extern ImGuiIO* imguiio;

extern bool guiShowing;

enum GuiScreen
{
    MainMenu,
    EscapeMenu,
    MatchStatePreparing
};

extern GuiScreen currentGuiScreen;

inline void initializeImGui(GLFWwindow* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    imguiio = &ImGui::GetIO();

    imguiio->IniFilename = nullptr;
    imguiio->LogFilename = nullptr;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

}

inline void renderImGui() {


        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

    if(guiShowing)
    {
        // Get the viewport size (entire screen size)
        ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

        // Set up the invisible window
        ImGui::SetNextWindowSize(viewportSize);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background

        // Disable window decorations
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoScrollbar |
                                       ImGuiWindowFlags_NoScrollWithMouse |
                                       ImGuiWindowFlags_NoCollapse |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus |
                                       ImGuiWindowFlags_NoNavFocus;

            ImGui::Begin("Invisible Window", nullptr, windowFlags);



            ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 1.0), "Tech Demo");

        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 textSize = ImGui::CalcTextSize("F: Use jetpack");

        // Set the cursor position to the bottom-left of the screen
        ImGui::SetCursorPos(ImVec2(5.0f, screenSize.y - textSize.y - 10.0f));

        ImGui::Text("F: Use jetpack");

        ImVec2 text2Size = ImGui::CalcTextSize("Shift: Sprint");

        // Set the cursor position to the bottom-left of the screen
        ImGui::SetCursorPos(ImVec2(5.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f));

        ImGui::Text("Shift: Sprint");

        ImVec2 text3Size = ImGui::CalcTextSize("Esc: Exit");

        // Set the cursor position to the bottom-left of the screen
        ImGui::SetCursorPos(ImVec2(5.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f - text3Size.y - 10.0f));

        ImGui::Text("Esc: Exit");


        ImGui::End(); // End the window
    }
        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}



#endif //IMGUISTUFF_H
