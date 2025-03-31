//
// Created by jack on 11/25/2024.
//

#ifndef IMGUISTUFF_H
#define IMGUISTUFF_H


#include "PrecompHeader.h"


#include "Server.h"

extern ImGuiIO* imguiio;

extern bool guiShowing;

enum class GuiScreen
{
    MainMenu,
    EscapeMenu,
    InGame,
    HostPort,
    Inventory,
    Orbit,
    SettingsMenu,
    Computer
};

extern GuiScreen currentGuiScreen;

enum class DGButtonType
{
    Bad1,
    Bad2,
    Good1,
    Good2,
    Stam1
};



float GetDPIScaling(GLFWwindow* window);

void SetImGuiScaling(GLFWwindow* window);

constexpr int BUTTONDIVS = 10;

std::vector<float> generateSubdividedQuad(int divisions);

void DrawCustomButtonBackground(ImVec2& pos, const ImVec2& size, DGButtonType type);

bool DGCustomButton(const char* label, DGButtonType type = DGButtonType::Good1, const ImVec2& size_arg = ImVec2(300, 70));

void initializeImGui(GLFWwindow* window);


static int ResizeStringCallback(ImGuiInputTextCallbackData* data);


void renderImGui();


#endif //IMGUISTUFF_H
