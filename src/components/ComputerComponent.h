//
// Created by jack on 3/26/2025.
//

#ifndef COMPUTERCOMPONENT_H
#define COMPUTERCOMPONENT_H
#include "NPPositionComponent.h"
#include "../Camera.h"
#include "../Shader.h"
#include "../TextEditor.h"
#include "../menupage/FullscreenKaleidoscope.h"
#include "../specialblocks/SpecialBlockInfo.h"

void drawTextEditor(TextEditor& editor);

class ComputerComponent {
public:
    TextEditor editor = {};

    GLuint fbo = 0, scrnTex = 0;
    int cwidth = 32, cheight = 22;
    int direction = 0;
    bool directionset = false;

    ComputerComponent() {
        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Python());
    }

    ///This initializes the gl objects lazily when called, so that the server wont have any gl objects
void renderEditorToFBO(TextEditor& editor, bool isRenderingOnly)
{
    // if (!ImGui::GetCurrentContext()) {
    //     throw std::runtime_error("No ImGui context active - initialize ImGui first");
    // }

    if (fbo == 0) {
        glGenFramebuffers(1, &fbo);
        if (fbo == 0) {
            throw std::runtime_error("Failed to generate framebuffer");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glGenTextures(1, &scrnTex);
        if (scrnTex == 0) {
            throw std::runtime_error("Failed to generate texture");
        }

        glBindTexture(GL_TEXTURE_2D, scrnTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cwidth, cheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scrnTex, 0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Framebuffer is not complete");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Save viewport and display size
    GLint originalViewport[4];
    glGetIntegerv(GL_VIEWPORT, originalViewport);
    // auto& io = ImGui::GetIO();
    // ImVec2 originalDisplaySize = io.DisplaySize;

    // // Set FBO state
    // io.DisplaySize = ImVec2((float)cwidth, (float)cheight);
    // io.WantCaptureMouse = false;
    // io.WantCaptureKeyboard = false;
    // io.WantTextInput = false;
    // io.MouseDrawCursor = false;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, cwidth, cheight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ImGui::NewFrame();
    // drawTextEditor(editor);
    // ImGui::Render();
    // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // ImGui::EndFrame(); // Reset ImGui state
        drawFullscreenKaleidoscope();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);
    //io.DisplaySize = originalDisplaySize;
}

    ComputerComponent& operator=(const ComputerComponent& other) = delete;
    ComputerComponent(ComputerComponent& other) = delete;
    ComputerComponent& operator=(ComputerComponent&& other) noexcept {
        if(this != &other) {
            editor = other.editor;
            fbo = other.fbo;
            scrnTex = other.scrnTex;
        }
        return *this;
    }
    ComputerComponent(ComputerComponent&& other) noexcept {
        editor = other.editor;
        fbo = other.fbo;
        scrnTex = other.scrnTex;
    }
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(direction);
    }
	~ComputerComponent()
    {
    }

    void drawAtPos(const glm::vec3& pos, jl::Camera& camera)
    {
        using namespace glm;

        renderEditorToFBO(editor, true);

        static jl::Shader shad = jl::Shader(R"glsl(
                #version 330 core
                layout(location = 0) in vec3 inPosition;
                layout(location = 1) in vec2 inTexCoord;
                out vec2 texCoord;

                uniform mat4 mvp;
                uniform vec3 pos;

                void main()
                {
                    texCoord = inTexCoord;
                    gl_Position = mvp * vec4(inPosition + pos, 1.0);
                }
            )glsl",
                    R"glsl(
                #version 330 core
                out vec4 FragColor;
                uniform sampler2D ourTexture;
                in vec2 texCoord;
                void main()
                {
                    FragColor = texture(ourTexture, texCoord);
                }
            )glsl",
            "compScreenShad");

        static std::vector<vec3> basevertices = {
            vec3(0.f, 0.3f, 0.f),
            vec3(0.f, 1.f, 0.f),
            vec3(1.f, 1.f, 0.f),

            vec3(1.f, 1.f, 0.f),
            vec3(1.f, 0.3f, 0.f),
            vec3(0.f, 0.3f, 0.f),
        };

        static std::vector<vec3> vertices;

        static bool verticesmade = false;
        if(!verticesmade)
        {
            vertices.insert(vertices.end(), basevertices.begin(), basevertices.end());
            auto rot1 = rotCoordsAroundYNeg90<glm::vec3>(vertices, 1);
            auto rot2 = rotCoordsAroundYNeg90<glm::vec3>(vertices, 2);
            auto rot3 = rotCoordsAroundYNeg90<glm::vec3>(vertices, 3);
            vertices.insert(vertices.end(), rot1.begin(), rot1.end());
            vertices.insert(vertices.end(), rot2.begin(), rot2.end());
            vertices.insert(vertices.end(), rot3.begin(), rot3.end());
            verticesmade = true;
        }
        static std::vector<vec2> texs = {
            vec2(1.f, 0.f),
            vec2(1.f, 1.f),
            vec2(0.f, 1.f),

            vec2(0.f, 1.f),
            vec2(0.f, 0.f),
            vec2(1.f, 0.f),

            vec2(1.f, 0.f),
            vec2(1.f, 1.f),
            vec2(0.f, 1.f),

            vec2(0.f, 1.f),
            vec2(0.f, 0.f),
            vec2(1.f, 0.f),

            vec2(1.f, 0.f),
            vec2(1.f, 1.f),
            vec2(0.f, 1.f),

            vec2(0.f, 1.f),
            vec2(0.f, 0.f),
            vec2(1.f, 0.f),

            vec2(1.f, 0.f),
            vec2(1.f, 1.f),
            vec2(0.f, 1.f),

            vec2(0.f, 1.f),
            vec2(0.f, 0.f),
            vec2(1.f, 0.f),

        };

        static GLuint vao = 0;
        static GLuint vbo = 0;
        static GLuint uvvbo = 0;

        if(vao == 0)
        {
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &uvvbo);

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);


            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, uvvbo);
            glBufferData(GL_ARRAY_BUFFER, texs.size() * sizeof(vec2), texs.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
        }

        glBindVertexArray(vao);
        glUseProgram(shad.shaderID);
        glUniformMatrix4fv(glGetUniformLocation(shad.shaderID, "mvp"), 1, GL_FALSE,
            glm::value_ptr(camera.mvp));
        glUniform3f(glGetUniformLocation(shad.shaderID, "pos"), pos.x, pos.y, pos.z);
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, scrnTex);
        glUniform1i(glGetUniformLocation(shad.shaderID, "ourTexture"), 0);
        glDrawArrays(GL_TRIANGLES, direction*6, 6);
        glBindVertexArray(0);

    }
};



inline void drawComputerScreensInReg(World* world, entt::registry& reg, jl::Camera& camera)
{
    auto view = reg.view<ComputerComponent, NPPositionComponent>();
    for (auto entity : view)
    {

        auto& pos = view.get<NPPositionComponent>(entity);
        auto& comp = view.get<ComputerComponent>(entity);

        //std::cout << "Should be drawing: " << pos.position.x << " " << pos.position.y << " " << pos.position.z << std::endl;
        if (!comp.directionset)
        {
            if (auto l = world->tryToGetReadLockOnDMs(); l != std::nullopt)
            {
                auto rawthere = world->getRawLocked(IntTup(pos.position.x, pos.position.y, pos.position.z));

                if ((rawthere & BLOCK_ID_BITS) == DG_COMPUTERBLOCK)
                {
                    auto bits = getDirectionBits(rawthere);
                    comp.directionset = true;
                    comp.direction = bits;
                }

            }
        }

        comp.drawAtPos(pos.position, camera);

    }
}



#endif //COMPUTERCOMPONENT_H
