//
// Created by jack on 11/25/2024.
//

#include "ImGuiStuff.h"

#include "DrawSky.h"
#include "FPSCounter.h"
#include "SharedVarsBetweenMainAndGui.h"
#include "LocalServerIOContext.h"
#include "LUTLoader.h"
#include "SunAndMoon.h"
//#include "TextEditor.h"
#include "TextEditor.h"
#include "Texture.h"
#include "TextureFace.h"
ImGuiIO* imguiio = nullptr;
bool guiShowing = true;
GuiScreen currentGuiScreen = GuiScreen::MainMenu;

float GetDPIScaling(GLFWwindow* window)
{
    // Get the monitor the window is currently on
    GLFWmonitor* monitor = glfwGetWindowMonitor(window);
    if (!monitor) {
        monitor = glfwGetPrimaryMonitor();
    }

    // Get the monitor's physical size and resolution
    int widthMM, heightMM;
    glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    int widthPx = mode->width;
    int heightPx = mode->height;

    // Calculate DPI (1 inch = 25.4 mm)
    float dpiX = static_cast<float>(widthPx) / (static_cast<float>(widthMM) / 25.4f);
    float dpiY = static_cast<float>(heightPx) / (static_cast<float>(heightMM) / 25.4f);

    // Use the average DPI for scaling
    return (dpiX + dpiY) / 2.0f / 96.0f; // 96 DPI is the baseline
}

void SetImGuiScaling(GLFWwindow* window)
{
    float systemScale = GetDPIScaling(window);

    ImGuiIO& io = ImGui::GetIO();

    int fbWidth, fbHeight, winWidth, winHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glfwGetWindowSize(window, &winWidth, &winHeight);

    float pixelRatio = (float)fbWidth / winWidth;
    io.DisplayFramebufferScale = ImVec2(pixelRatio, pixelRatio);

    io.FontGlobalScale = systemScale;

    ImGuiStyle defaultStyle;
    ImGuiStyle& style = ImGui::GetStyle();
    style = defaultStyle;
    style.ScaleAllSizes(systemScale);
}

std::vector<float> generateSubdividedQuad(int divisions)
{
    std::vector<float> vertices;
    float step = 1.0f / divisions;

    for (int y = 0; y < divisions; ++y) {
        for (int x = 0; x < divisions; ++x) {
            float x0 = x * step;
            float y0 = y * step;
            float x1 = (x + 1) * step;
            float y1 = (y + 1) * step;

            float s0 = x0;
            float t0 = 1.0f - y0; // Flip Y coordinate
            float s1 = x1;
            float t1 = 1.0f - y1; // Flip Y coordinate

            // First triangle
            vertices.insert(vertices.end(), {x0, y0, s0, t0});
            vertices.insert(vertices.end(), {x1, y0, s1, t0});
            vertices.insert(vertices.end(), {x1, y1, s1, t1});

            // Second triangle
            vertices.insert(vertices.end(), {x1, y1, s1, t1});
            vertices.insert(vertices.end(), {x0, y1, s0, t1});
            vertices.insert(vertices.end(), {x0, y0, s0, t0});
        }
    }

    return vertices;
}

void DrawCustomButtonBackground(ImVec2& pos, const ImVec2& size, DGButtonType type)
{

    ImVec2 windowStart = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();


    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    static jl::Texture textures[5] = {
        jl::Texture("resources/gui/bad1.png"),
        jl::Texture("resources/gui/bad2.png"),
        jl::Texture("resources/gui/good1.png"),
        jl::Texture("resources/gui/good2.png"),
        jl::Texture("resources/gui/stam1.png"),
    };

    static jl::Shader shader(
        R"glsl(
            #version 330 core

layout (location = 0) in vec2 aPos;     // 2D position (0.0 to 1.0)
layout (location = 1) in vec2 aTexCoord; // Texture coordinates

out vec2 TexCoord;

uniform vec2 uPos;  // Position of the quad in NDC
uniform vec2 uSize; // Size of the quad in NDC
uniform float uTime; // Time uniform for animation

// Hash function for pseudo-random noise
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

// 2D pseudo-random noise function (Perlin-like)
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    // Four corners
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    // Smooth interpolation
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

void main()
{
    // Scale and translate the vertex position
    vec2 scaledPos = aPos * uSize + uPos;
    gl_Position = vec4(scaledPos, 0.0, 1.0); // Already in NDC

    // Generate seamless UV turbulence using final screen-space position
    float displacement = noise(scaledPos * 10.0 + vec2(uTime * 0.7, uTime * 0.5)) * 0.05;

    // Apply displacement to UVs
    TexCoord = aTexCoord + vec2(displacement, displacement);
}
        )glsl",
        R"glsl(
            #version 330 core

            in vec2 TexCoord;

            out vec4 FragColor;

            uniform sampler2D texture1;

            uniform float bright;
            uniform vec2 ndcMin;
            uniform vec2 ndcMax;
            void main()
            {
                FragColor = texture(texture1, TexCoord) * bright;
                // In fragment shader
                if (gl_FragCoord.x < ndcMin.x || gl_FragCoord.x > ndcMax.x ||
                    gl_FragCoord.y > ndcMax.y || gl_FragCoord.y < ndcMin.y) { // Note Y comparison direction
                    discard;
                }
            }
        )glsl",
        "customButtonShaderYo"
    );

    glUseProgram(shader.shaderID);

    static GLuint vao = 0;
    static GLuint vbo = 0;
    if (vao == 0)
    {
        std::vector<float> vertices = generateSubdividedQuad(BUTTONDIVS);

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

    // Get the screen dimensions (in pixels)
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;

    // Convert ImGui pixel coordinates to NDC
    ImVec2 ndcPos;
    ndcPos.x = (pos.x / screenSize.x) * 2.0f - 1.0f; // Convert X to NDC
    ndcPos.y = 1.0f - (pos.y / screenSize.y) * 2.0f; // Convert Y to NDC (flip Y-axis)

    ImVec2 ndcSize;
    ndcSize.x = (size.x / screenSize.x) * 2.0f; // Convert width to NDC
    ndcSize.y = (size.y / screenSize.y) * 2.0f; // Convert height to NDC

    ndcPos.y -= ndcSize.y;

    // Pass pos and size as uniforms
    static GLint posUniform = glGetUniformLocation(shader.shaderID, "uPos");
    static GLint sizeUniform = glGetUniformLocation(shader.shaderID, "uSize");

    static GLint timeUni = glGetUniformLocation(shader.shaderID, "uTime");
    static GLint ndcm = glGetUniformLocation(shader.shaderID, "ndcMin");
    static GLint ndcma = glGetUniformLocation(shader.shaderID, "ndcMax");
    static GLint brightUni = glGetUniformLocation(shader.shaderID, "bright");
    glUniform2f(posUniform, ndcPos.x, ndcPos.y);
    glUniform2f(sizeUniform, ndcSize.x, ndcSize.y);

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        glUniform1f(timeUni, glfwGetTime() * 8.0f);
        glUniform1f(brightUni, 1.1f);
    } else
    {

        glUniform1f(timeUni, glfwGetTime() * 3.0f);
        glUniform1f(brightUni, 1.0f);
    }

    // Bind texture and draw
    textures[(int)type].bind_to_unit(4);
    static GLint texPos = glGetUniformLocation(shader.shaderID, "texture1");
    glUniform1i(texPos, 4);

    // Convert your windowStart and windowSize to pixel coordinates for clipping
    ImVec2 clipMin = windowStart;
    ImVec2 clipMax = ImVec2(windowStart.x + windowSize.x, windowStart.y + windowSize.y);

    // Then pass these to the shader
    glUniform2f(ndcma, clipMax.x, clipMax.y);
    glUniform2f(ndcm, clipMin.x, clipMin.y);

    glDrawArrays(GL_TRIANGLES, 0, 6 * (BUTTONDIVS  * BUTTONDIVS));
    if (depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

bool DGCustomButton(const char* label, DGButtonType type, const ImVec2& size_arg)
{

    ImGuiStyle& style = ImGui::GetStyle();

    // Extract visible label (stop at "##")
    const char* label_display_end = label;
    while (*label_display_end != '\0' && !(label_display_end[0] == '#' && label_display_end[1] == '#')) {
        label_display_end++;
    }

    // Calculate text size
    ImVec2 label_size = ImGui::CalcTextSize(label, label_display_end);

    auto realsizex = size_arg.x;


    float scale = imguiio->FontGlobalScale;

    if (size_arg.x * scale < label_size.x)
        realsizex = (label_size.x + 100) / scale;


    // Calculate button size (respecting FramePadding)
    ImVec2 size = ImVec2(
        realsizex > 0.0f ? realsizex : label_size.x + style.FramePadding.x * 2.0f,
        size_arg.y > 0.0f ? size_arg.y : label_size.y + style.FramePadding.y * 2.0f
    );

    size = ImVec2(size.x * scale, size.y * scale);

    ImGui::PushID(label);
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // InvisibleButton with proper size
    ImGui::InvisibleButton(label, size);

    bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);

    // Draw your custom background
    DrawCustomButtonBackground(pos, size, type);

    // Position text CORRECTLY (match standard Button alignment)
    ImVec2 text_pos = ImVec2(
        pos.x + style.FramePadding.x,
        pos.y + (size.y - label_size.y) * 0.5f // Center vertically
    );

    // Render text directly to avoid cursor disruption
    ImGui::GetWindowDrawList()->AddText(text_pos,
                                        ImGui::GetColorU32(ImGuiCol_Text),
                                        label, label_display_end);

    ImGui::PopID();
    return clicked;
}

void initializeImGui(GLFWwindow* window)
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

int ResizeStringCallback(ImGuiInputTextCallbackData* data)
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

void renderImGui()
{


    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if(guiShowing)
    {

        if(currentGuiScreen == GuiScreen::Computer || currentGuiScreen == GuiScreen::InGame)
        {
            auto now = std::chrono::steady_clock::now();
            auto & messages = theScene.messages;
            // Remove expired messages
            messages.erase(std::remove_if(messages.begin(), messages.end(),
                          [now](const ChatMessage& msg) {
                              return std::chrono::duration_cast<std::chrono::seconds>(now - msg.timestamp).count() > 10;
                          }),
                          messages.end());

            // Set the chat window position and size
            ImVec2 window_pos = ImVec2(50, ImGui::GetIO().DisplaySize.y - 100);  // Bottom-left corner
            ImVec2 window_size = ImVec2(400, 150);  // Width and height

            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

            // Transparent window background
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.3f));

            if (ImGui::Begin("ChatWindow", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNavFocus)) {
                ImGui::SetWindowFontScale(1.2f);  // Adjust text size

                // Render messages from newest to oldest
                for (int i = static_cast<int>(messages.size()) - 1; i >= 0; --i) {
                    ImGui::TextWrapped("%s", messages[i].text.c_str());
                }
            }
            ImGui::End();

            ImGui::PopStyleColor();
        }


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
        case GuiScreen::Inventory:
            {
                imguiInventory(theScene.our<InventoryComponent>().inventory);
                break;
            }
        case GuiScreen::Computer:
            {
                if (theScene.currentEditor)
                {
                    drawTextEditor(*theScene.currentEditor);
                    ImGui::SetCursorPosY(700.0f);
                    if(DGCustomButton("Execute", DGButtonType::Good1))
                    {
                        theScene.pythonContext.exec(theScene.currentEditor->GetText());
                    }
                }

                break;
            }
        case GuiScreen::SettingsMenu:
            {
                if(DGCustomButton("Back", DGButtonType::Good2))
                {
                    theScene.saveSettings();
                    if(theScene.worldReceived.load()) //if we're in-game, refresh rend dist and amb occl (rebuild all chunks)
                    {
                        theScene.worldRenderer->setRenderDistance(theScene.rendDistSelection, &theScene.our<jl::Camera>(), theScene.world);
                        currentGuiScreen = GuiScreen::EscapeMenu;
                    } else
                    {
                        currentGuiScreen = GuiScreen::MainMenu;
                    }
                }
                if (ImGui::SliderFloat("Mouse Sensitivity", &theScene.settings.mouseSensitivity, 0.0f, 2.0f))
                {
                    theScene.saveSettings();
                }

                if(ImGui::SliderInt("Render Distance", &theScene.rendDistSelection, 2, 64))
                {

                    theScene.saveSettings();

                    // if(theScene.worldReceived.load()) //if we're in-game, refresh rend dist and amb occl (rebuild all chunks)
                    // {
                    //     theScene.worldRenderer->setRenderDistance(theScene.rendDistSelection, &theScene.our<jl::Camera>(), theScene.world);
                    // }
                }

                if (ImGui::Checkbox("Ambient Occlusion", &ambOccl))
                {
                    theScene.saveSettings();
                }

                if(DGCustomButton("Toggle Fullscreen", DGButtonType::Good1))
                {
                    toggleFullscreen(theScene.window);
                }

                if (ImGui::SliderFloat("Music Volume", &theScene.settings.musicVol, 0.0f, 1.0f))
                {
                    theScene.saveSettings();
                    alSourcef(theScene.musicSource, AL_GAIN, theScene.settings.musicVol);
                }

                break;
            }
        case GuiScreen::HostPort:
            //drawFullscreenKaleidoscope();
            if (DGCustomButton("Back to main menu", DGButtonType::Bad1))
            {
                currentGuiScreen = GuiScreen::MainMenu;
            }
            ImGui::Text("Enter a TCP-forwarded port for hosting (or leave blank for default 25000):");
            ImGui::InputText("Port", theScene.localServerPort.data(), theScene.localServerPort.capacity(), ImGuiInputTextFlags_CallbackResize, ResizeStringCallback, &theScene.localServerPort);
            if (DGCustomButton("Start server and join", DGButtonType::Good2))
            {
                try
                {

                    std::string ip = "127.0.0.1";

                    if (theScene.localServerPort.size() == 0)
                    {
                        theScene.localServerPort = "25000";
                    }


                    launchLocalServer(std::stoi(theScene.localServerPort));

                    std::this_thread::sleep_for(std::chrono::seconds(1));


                    theScene.enableMultiplayer();

                    if(connectToServer(ip.c_str(), theScene.localServerPort.c_str()))
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

                    }
                } catch (std::exception& e)
                {

                }

            }
            break;
        case GuiScreen::MainMenu:
            //drawFullscreenKaleidoscope();

        {
            static auto lasttime = glfwGetTime();
            auto time = glfwGetTime();
            auto deltaTime = time - lasttime;
            //dgDrawSky(glm::vec3(0.0), luttexture, *theScene.world, 700.0f, );
            drawSunAndMoon(theScene.menuCamera, deltaTime, 100.0f, glm::vec3(0.0f));
        }

            

            {

            ImGui::PushFont(theScene.font_title);
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Distant Garden");
                ImGui::PopFont();
            }

            {
                ALint musicSourceState;
                alGetSourcei(theScene.musicSource, AL_SOURCE_STATE, &musicSourceState);
                if (musicSourceState != AL_PLAYING)
                {
                    theScene.playSong(sounds.at((int)SoundBuffers::SONG1), true);
                }
            }

            if (ImGui::InputText("Server address", theScene.serverAddress.data(), theScene.serverAddress.capacity(), ImGuiInputTextFlags_CallbackResize, ResizeStringCallback, &theScene.serverAddress))
            {
                theScene.saveSettings();
            }
            if (DGCustomButton("Connect to server", DGButtonType::Good2))
            {

                size_t colonPos = theScene.serverAddress.find(':');

                if (colonPos != std::string::npos) {
                    std::string ip = theScene.serverAddress.substr(0, colonPos);
                    std::string port = theScene.serverAddress.substr(colonPos + 1);



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

                    }
                } else {

                }



            }
            if (DGCustomButton("Host", DGButtonType::Good2))
            {

                currentGuiScreen = GuiScreen::HostPort;

            }
            if (DGCustomButton("Settings", DGButtonType::Good1))
            {
                currentGuiScreen = GuiScreen::SettingsMenu;
            }
            if (DGCustomButton("Exit game", DGButtonType::Bad2))
            {
                glfwSetWindowShouldClose(theScene.window, true);
            }


            break;
        case GuiScreen::EscapeMenu:
            {





                if (DGCustomButton("Back to game", DGButtonType::Good2))
                {
                    currentGuiScreen = GuiScreen::InGame;
                }
                if (DGCustomButton("Settings", DGButtonType::Good1))
                {
                    currentGuiScreen = GuiScreen::SettingsMenu;
                }
                std::string leavelabel;
                if (localserver_running.load())
                {
                    leavelabel = "Leave to main menu and save/close server";
                } else
                {
                    leavelabel = "Disconnect and leave to main menu";
                }
                if (DGCustomButton(leavelabel.c_str(), DGButtonType::Bad1))
                {
                    exitWorld(&theScene);
                    uncaptureMouse(&theScene);
                    if(theScene.multiplayer)
                    {
                        tsocket.close();
                    }
                    endLocalServerIfRunning();
                    currentGuiScreen = GuiScreen::MainMenu;
                }
                if (DGCustomButton("Exit to desktop", DGButtonType::Bad2))
                {
                    exitWorld(&theScene);
                    uncaptureMouse(&theScene);
                    if(theScene.multiplayer)
                    {
                        tsocket.close();
                    }
                    endLocalServerIfRunning();

                    glfwSetWindowShouldClose(theScene.window, true);
                }
                break;
            }
        case GuiScreen::InGame:

            ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 1.0), "dg alpha team 6");

            for (int i = 0; i < theScene.our<MovementComponent>().stamCount; i++)
            {
                DGCustomButton((std::string("##stam") + std::to_string(i)).c_str(), DGButtonType::Stam1, ImVec2(15, 40));
            }


            ImVec2 screenSize = ImGui::GetIO().DisplaySize;

            ImVec2 invTileDisplaySize = ImVec2((screenSize.x / 40.0f), (screenSize.x / 40.0f)); // Size of displayed image


            ImVec2 startInvRow = ImVec2((screenSize.x / 2.f) - ((invTileDisplaySize.x + 20.0f) * 5.f)/2.f, screenSize.y - (screenSize.y / 20.0f));

            ImVec2 prevPos = ImGui::GetCursorPos();

            auto & inv = theScene.our<InventoryComponent>();

            int highlightedSlot = (int)theScene.our<InventoryComponent>().currentHeldInvIndex;

            for (int i = 0; i < 5; i++) {
                ImVec2 start = startInvRow + ImVec2(i * ((screenSize.x / 40.0f) + 20.0f), 0);
                ImGui::SetCursorPos(start);

                if(i == highlightedSlot) {
                    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(start.x - 10.0f, start.y - 10.0f), ImVec2(start.x + invTileDisplaySize.x + 10.0f,
                         start.y + invTileDisplaySize.x + 10.0f), IM_COL32_WHITE);
                }

                const auto tex = TEXS.at(inv.inventory.inventory[i].block).at(0);
                TextureFace face(tex.first, tex.second);

                ImGui::Image((ImTextureID)theScene.worldtex, invTileDisplaySize, ImVec2(face.bl.x, face.bl.y), ImVec2(face.tr.x, face.tr.y));


                ImGui::SetCursorPos(start);

                ImGui::Text(std::to_string(inv.inventory.inventory[i].count).c_str());

                ImGui::SetCursorPos(start);

                DGCustomButton((std::string("##hudinvrowback") + std::to_string(i)).c_str(), DGButtonType::Good1, invTileDisplaySize);


            }

            ImGui::SetCursorPos(prevPos);


            if (!theScene.bulkPlaceGizmo->active && !theScene.vmStampGizmo->active)
            {
                if (theScene.showingControls)
                {
                    const char* s1 = "F3: Hide help";
                    ImVec2 textSize = ImGui::CalcTextSize(s1);

                    // Set the cursor position to the bottom-left of the screen
                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f));

                    ImGui::Text(s1);


                    const char* s2 = "Shift: Sprint";
                    ImVec2 text2Size = ImGui::CalcTextSize(s2);

                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f));

                    ImGui::Text(s2);

                    const char* s3 = "Esc: Exit";
                    ImVec2 text3Size = ImGui::CalcTextSize(s3);

                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f - text3Size.y - 10.0f));

                    ImGui::Text(s3);

                    const char* s4 = "H: Hover mode";
                    ImVec2 textSize0 = ImGui::CalcTextSize(s4);

                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f - text3Size.y - 10.0f - textSize0.y - 10.0f));

                    ImGui::Text(s4);

                    const char* s5 = "B: Toggle Build Mode";
                    ImVec2 textSize4 = ImGui::CalcTextSize(s5);

                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f - text3Size.y - 10.0f - textSize0.y - 10.0f - textSize4.y - 10.0f));

                    ImGui::Text(s5);


                    const char* s6 = "Middle-click: Pick block";
                    ImVec2 textSize6 = ImGui::CalcTextSize(s6);

                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f - text3Size.y - 10.0f - textSize0.y - 10.0f - textSize4.y - 10.0f - textSize6.y - 10.0f));

                    ImGui::Text(s6);


                    const char* s7 = "F: Use jetpack";
                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - (( textSize.y + 10.0f) * 7)));

                    ImGui::Text(s7);

                    const char* s8 = "V: Toggle voxel model stamp mode";
                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - (( textSize.y + 10.0f) * 8)));

                    ImGui::Text(s8);

                    const char* s9 = "E: Open/Close Inventory";
                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - (( textSize.y + 10.0f) * 9)));

                    ImGui::Text(s9);

                    std::string fps = std::to_string(DG_FPS) + " FPS";
                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - (( textSize.y + 10.0f) * 10)));

                    ImGui::Text(fps.c_str());





                    static std::string coordinatesString = "";
                    glm::vec3 pp = theScene.our<jl::Camera>().transform.position;
                    coordinatesString = std::string("X: ") + std::to_string(pp.x) + ", Y: " + std::to_string(pp.y) + ", Z: " + std::to_string(pp.z);

                    ImGui::SetCursorPos(ImVec2(10.0f, 10.0f  + textSize.y));

                    ImGui::Text(coordinatesString.c_str());
                } else
                {
                    const char* s1 = "F3: Toggle help";
                    ImVec2 textSize = ImGui::CalcTextSize(s1);

                    // Set the cursor position to the bottom-left of the screen
                    ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f));

                    ImGui::Text(s1);
                }

            } else if (theScene.bulkPlaceGizmo->active)
            {
                const char* s1 = "Right click: Confirm and place";
                ImVec2 textSize = ImGui::CalcTextSize(s1);

                // Set the cursor position to the bottom-left of the screen
                ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f));

                ImGui::Text(s1);


                const char* s2 = "B: Toggle Mode / Exit Build Mode";
                ImVec2 text2Size = ImGui::CalcTextSize(s2);

                ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f));

                ImGui::Text(s2);

                const char* s3 = "Yellow Outline: Hollow";
                ImVec2 text3Size = ImGui::CalcTextSize(s3);

                ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f - text3Size.y - 10.0f));

                ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0),s3);

                const char* s4 = "Red Outline: Solid";
                ImVec2 textSize0 = ImGui::CalcTextSize(s4);

                ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f - text3Size.y - 10.0f - textSize0.y - 10.0f));

                ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0),s4);



            } else if (theScene.vmStampGizmo->active)
            {
                const char* s1 = "Right click: Stamp model";
                ImVec2 textSize = ImGui::CalcTextSize(s1);

                // Set the cursor position to the bottom-left of the screen
                ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f));

                ImGui::Text(s1);


                const char* s2 = "V: Toggle Mode / Exit Stamp Mode";
                ImVec2 text2Size = ImGui::CalcTextSize(s2);

                ImGui::SetCursorPos(ImVec2(10.0f, screenSize.y - textSize.y - 10.0f - text2Size.y - 10.0f));

                ImGui::Text(s2);



            }




            imguiio->WantCaptureMouse = false;
            break;

        }


        ImGui::End(); // End the window
    }
    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}
