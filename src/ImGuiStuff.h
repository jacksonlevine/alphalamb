//
// Created by jack on 11/25/2024.
//

#ifndef IMGUISTUFF_H
#define IMGUISTUFF_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "SharedVarsBetweenMainAndGui.h"

extern ImGuiIO* imguiio;

extern bool guiShowing;

enum GuiScreen
{
    MainMenu,
    EscapeMenu,
    InGame
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


static int ResizeStringCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        std::string* my_str = (std::string*)(data->UserData);
        IM_ASSERT(my_str->data() == data->Buf);
        my_str->resize(data->BufSize);
        data->Buf = my_str->data();
    }
    return 0;
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


        switch (currentGuiScreen)
        {
        case GuiScreen::MainMenu:
                ImGui::InputText("Server address", theScene.serverAddress.data(), theScene.serverAddress.capacity(), ImGuiInputTextFlags_CallbackResize, ResizeStringCallback, &theScene.serverAddress);
                if (ImGui::Button("Connect to server"))
                {

                    size_t colonPos = theScene.serverAddress.find(':');

                    if (colonPos != std::string::npos) {
                        std::string ip = theScene.serverAddress.substr(0, colonPos);
                        std::string port = theScene.serverAddress.substr(colonPos + 1);
                        std::cout << "IP: " << ip << "\n";
                        std::cout << "Port: " << port << "\n";

                        theScene.enableMultiplayer();

                        if(connectToServer(ip.c_str(), port.c_str()))
                        {
                            while (!theScene.worldReceived.load())
                            {
                                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            enterWorld(&theScene);
                            currentGuiScreen = GuiScreen::InGame;
                        } else
                        {
                            std::cout << "Couldn't connect \n";
                        }
                    } else {
                        std::cout << "Invalid format.\n";
                    }



                }
                if (ImGui::Button("Exit game"))
                {
                    glfwSetWindowShouldClose(theScene.window, true);
                }
                break;
            case GuiScreen::EscapeMenu:

                if (ImGui::Button("Back to game"))
                {
                    currentGuiScreen = GuiScreen::InGame;
                }
                if (ImGui::Button("Leave to main menu"))
                {
                    exitWorld(&theScene);
                    uncaptureMouse(&theScene);
                    if(theScene.multiplayer)
                    {
                        tsocket.close();
                    }
                    currentGuiScreen = GuiScreen::MainMenu;
                }
                if (ImGui::Button("Exit to desktop"))
                {
                    exitWorld(&theScene);
                    glfwSetWindowShouldClose(theScene.window, true);
                }
                break;
            case GuiScreen::InGame:

                ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 1.0), "dg 0.0.91a");

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

                imguiio->WantCaptureMouse = false;
                break;

        }


        ImGui::End(); // End the window
    }
        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}



#endif //IMGUISTUFF_H
