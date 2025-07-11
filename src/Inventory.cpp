//
// Created by jack on 2/17/2025.
//

#include "Inventory.h"

#include "Client.h"
#include "ImGuiStuff.h"
#include "Texture.h"
#include "TextureFace.h"
#include "DrawBillboard2D.h"
void imguiInventory(Inventory& inv)
{
    // static bool testset = false;
    // if (!testset)
    // {
    //     inv.setSlot(3, 2, InventorySlot{
    //     .block = 5, .count = 10});
    //     inv.setSlot(4, 2, InventorySlot{
    //     .block = 1, .count = 1, .isItem = true});
    //     testset = true;
    // }

    ImVec2 screenSize = ImGui::GetIO().DisplaySize;

    ImVec2 backgroundSize = ImVec2(screenSize.x * 0.75f, screenSize.y* 0.75f);

    ImVec2 invTileDisplaySize = ImVec2(50, 50); // Size of displayed image


    ImVec2 pos = ImVec2((screenSize.x - backgroundSize.x) * 0.5f, (screenSize.y - backgroundSize.y) * 0.5f);


    static jl::Texture invBackground("resources/invbackground.png");

    // Draw background image FIRST using direct OpenGL (before ImGui)
    {
        GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
        glDisable(GL_DEPTH_TEST);

        // Use the same shader you're using for buttons
        static jl::Shader shader(
            R"glsl(
                #version 330 core
                layout (location = 0) in vec2 aPos;
                layout (location = 1) in vec2 aTexCoord;
                out vec2 TexCoord;
                uniform vec2 uPos;
                uniform vec2 uSize;
                uniform float uTime;
                void main()
                {
                    vec2 scaledPos = aPos * uSize + uPos;
                    gl_Position = vec4(scaledPos, 0.0, 1.0);
                    TexCoord = aTexCoord;
                }
            )glsl",
            R"glsl(
                #version 330 core
                in vec2 TexCoord;
                out vec4 FragColor;
                uniform sampler2D texture1;
                uniform float bright;
                void main()
                {
                    FragColor = texture(texture1, TexCoord);
                }
            )glsl",
            "backgroundShader"
        );

        glUseProgram(shader.shaderID);

        // Use the same VAO/VBO setup you're using for buttons
        static GLuint vao = 0;
        static GLuint vbo = 0;
        if (vao == 0)
        {
            std::vector<float> vertices = generateSubdividedQuad(2); // Simpler subdivision for background

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float)*2));
        }

        glBindVertexArray(vao);

        ImVec2 ndcPos;
        ndcPos.x = (pos.x / screenSize.x) * 2.0f - 1.0f;
        ndcPos.y = 1.0f - (pos.y / screenSize.y) * 2.0f;

        ImVec2 ndcSize;
        ndcSize.x = (backgroundSize.x / screenSize.x) * 2.0f;
        ndcSize.y = (backgroundSize.y / screenSize.y) * 2.0f;

        ndcPos.y -= ndcSize.y;

        // Set uniforms
        GLint posUniform = glGetUniformLocation(shader.shaderID, "uPos");
        GLint sizeUniform = glGetUniformLocation(shader.shaderID, "uSize");
        GLint timeUni = glGetUniformLocation(shader.shaderID, "uTime");
        GLint brightUni = glGetUniformLocation(shader.shaderID, "bright");
        GLint texPos = glGetUniformLocation(shader.shaderID, "texture1");

        glUniform2f(posUniform, ndcPos.x, ndcPos.y);
        glUniform2f(sizeUniform, ndcSize.x, ndcSize.y);
        glUniform1f(timeUni, 0.0f); // No animation for background
        glUniform1f(brightUni, 1.0f);

        // Bind texture and draw
        invBackground.bind_to_unit(0);
        glUniform1i(texPos, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6 * 4); // 6 vertices per quad, 4 quads

        // Restore state
        if (depthTestEnabled)
            glEnable(GL_DEPTH_TEST);
    }





    ImGui::SetNextWindowPos(ImVec2(pos.x + backgroundSize.x*0.2f, pos.y + backgroundSize.y*0.2f));
    ImGui::SetNextWindowSize(ImVec2(backgroundSize.x - (backgroundSize.x*0.4f), backgroundSize.y - (backgroundSize.y*0.4f)));

    //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));



    ImGui::Begin("Background", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar
                                        | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar );
    ImGui::BeginGroup();
        ImGui::BeginGroup();
            for (int j = 0; j < INVHEIGHT; j++)
            {
                for (int i = 0; i < INVWIDTH; i++)
                {
                    int index = j*INVWIDTH + i;
                    ImGui::PushID(index);
                    if (i > 0) ImGui::SameLine(0.f, 0.f);


                    auto cursorspot = ImGui::GetCursorPos();

                    auto& slot = inv.getSlot(i, j);
                    std::string label = slot.empty() ? std::string("##inv") + std::to_string(index)  : "  " + std::to_string(slot.count);

                    ImGui::SetCursorPos(cursorspot);

                    if (slot.isItem)
                    {
                        const auto tex = ITEMTEXS.at(inv.inventory[index].block).at(0);
                        TextureFace face(tex.first, tex.second);

                        ImGui::Image((ImTextureID)theScene.itemstex, invTileDisplaySize, ImVec2(face.bl.x, face.bl.y), ImVec2(face.tr.x, face.tr.y));

                    } else
                    {
                        const auto tex = TEXS.at(inv.inventory[index].block).at(0);
                        TextureFace face(tex.first, tex.second);

                        ImGui::Image((ImTextureID)theScene.worldtex, invTileDisplaySize, ImVec2(face.bl.x, face.bl.y), ImVec2(face.tr.x, face.tr.y));

                    }


                    ImGui::SetCursorPos(cursorspot);

                    bool isEquip = Inventory::isEquipSlot(i,j);
                    bool mouseHeldItemEquippable = equippable(static_cast<ItemName>(inv.mouseHeldItem.block));


                    //
                    // if (isEquip)
                    // {
                    //     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0,0.3,0.3,0.7));
                    // }

                    auto bt = isEquip ? DGButtonType::Bad1 : DGButtonType::Good1;

                    if (DGCustomButton(label.c_str(), bt, invTileDisplaySize, 1.f, false, true))
                    {
                        if (slot.block == inv.mouseHeldItem.block && slot.block != 0 && inv.mouseHeldItem.block != 0)
                        {
                            pushToMainToNetworkQueue(RequestStackSlotsToDest{
                                .sourceID = theScene.settings.clientUID, .destinationID = theScene.settings.clientUID, .myPlayerIndex = theScene.myPlayerIndex,
                                .sourceIndex = 0, .destinationIndex = (uint8_t)inv.getIndex(i, j), .mouseSlotS = true, .mouseSlotD = false});

                        } else
                        {
                            bool validswap = true;
                            if (isEquip && !mouseHeldItemEquippable)
                            {
                                validswap = false;
                            }
                            if (validswap)
                            {
                                //std::cout << "Heyoo" << std::endl;
                                pushToMainToNetworkQueue(RequestInventorySwap{
                                .sourceID = theScene.settings.clientUID, .destinationID = theScene.settings.clientUID, .myPlayerIndex = theScene.myPlayerIndex,
                                .sourceIndex = 0, .destinationIndex = (uint8_t)inv.getIndex(i, j), .mouseSlotS = true, .mouseSlotD = false});

                                // auto mouseslot = inv.mouseHeldItem;
                                // inv.mouseHeldItem = slot;
                                // slot = mouseslot;
                            }
                        }
                        theScene.shitICanMake = getShitCanMake(theScene.our<InventoryComponent>().inventory);
                    }

                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    {
                        if (isEquip && !mouseHeldItemEquippable) {
                            ImGui::SetTooltip("Cannot equip this item.");
                        } else if (isEquip && mouseHeldItemEquippable && inv.mouseHeldItem.block != 0) {
                            ImGui::SetTooltip("Equip this item.");
                        } else if (!slot.empty()) {
                            if (slot.isItem) {
                                ImGui::SetTooltip(ToString(static_cast<ItemName>(slot.block)));
                            } else {
                                ImGui::SetTooltip(ToString(static_cast<MaterialName>(slot.block)));
                            }
                        }
                    }
                    // if (isEquip)
                    // {
                    //     ImGui::PopStyleColor();
                    // }
                    ImGui::PopID();
                }
                ImGui::NewLine();
            }
        ImGui::EndGroup();



        ImGui::SameLine(0.0f, 0.0f);

        ImGui::BeginGroup();
            ImGui::BeginGroup();
            auto curspos = ImGui::GetCursorScreenPos();
            auto curspos2 = ImGui::GetCursorPos();

            ImGui::GetWindowDrawList()->AddRect(curspos, curspos + ImVec2(160, 300), ImColor(ImVec4(0.f, 0.f, 0.f, 1.0)), 3, ImDrawFlags_RoundCornersAll, 2);
            ImGui::Dummy(ImVec2(160, 300));

            ImGui::SetCursorPos(curspos2);
            draw2DBillboard(ImVec2(300, 300), 70.0f);

            ImGui::EndGroup();

            ImGui::NewLine();

            ImGui::BeginGroup();
                static float heartbeat_pattern[] = {
                    0.0f, 0.1f, 0.0f, -0.1f, 0.0f, 0.2f, 0.8f, 1.0f, 0.3f, -0.2f,
                    0.0f, 0.1f, 0.5f, 0.2f, 0.0f, -0.1f, 0.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
                }; // 30 samples per heartbeat
                static const int heartbeat_length = sizeof(heartbeat_pattern) / sizeof(heartbeat_pattern[0]);

                static float values[90] = {};
                static int values_offset = 0;
                static double refresh_time = 0.0;
    //std::cout << " AY: " << theScene.our<HealthComponent>().heartRate << " " << theScene.our<HealthComponent>().health << std::endl;
                float tempo = theScene.our<HealthComponent>().heartRate;
                static int pattern_index = 0;

                if (refresh_time == 0.0)
                    refresh_time = ImGui::GetTime();

                while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate
                {
                    values[values_offset] = heartbeat_pattern[pattern_index];
                    values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);

                    // Advance through the heartbeat pattern based on tempo
                    static float pattern_position = 0.0f;
                    pattern_position += tempo;
                    pattern_index = ((int)pattern_position) % heartbeat_length;

                    refresh_time += 1.0f / 60.0f;
                }

                // Display the plot
                {
                    ImGui::PlotLines("##Heartbeat", values, IM_ARRAYSIZE(values), values_offset, "", -0.5f, 1.2f, ImVec2(160.0f, 40.0f));
                }

                ImGui::NewLine();
                //Health bar
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
                            ImGui::ProgressBar(theScene.our<HealthComponent>().health/100.0f, ImVec2(160.0f, 20.0f), "Health");
                ImGui::PopStyleColor(2);

            ImGui::EndGroup();

        ImGui::EndGroup();
    ImGui::EndGroup();

    ImGui::NewLine();
    ImGui::BeginGroup();

    for (auto & i : theScene.shitICanMake)
    {

        if (recipes.at(i).second.isItem)
        {
            if (DGCustomButton(ToString((ItemName)recipes.at(i).second.mat), DGButtonType::Bad2, ImVec2(200, 50)))
            {
                pushToMainToNetworkQueue(DoRecipeOnMyInv{
                    .myPlayerIndex = theScene.myPlayerIndex,
                    .recipeIndex = i
                });
            }
        } else
        {
            if (DGCustomButton(ToString((MaterialName)recipes.at(i).second.mat), DGButtonType::Bad2, ImVec2(200, 50)))
            {
                pushToMainToNetworkQueue(DoRecipeOnMyInv{
                    .myPlayerIndex = theScene.myPlayerIndex,
                    .recipeIndex = i
                });
            }
        }

    }

    ImGui::EndGroup();

    ImGui::End();
}
