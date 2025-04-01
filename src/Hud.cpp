//
// Created by jack on 11/30/2024.
//

#include "Hud.h"

#include "Shader.h"
#include "Texture.h"

Hud::Hud() {
    jl::Texture cht("resources/crosshair.png");

    texture = cht.id;
    glGenBuffers(1, &vbo);
}

GLuint hudVAO = 0;


jl::Shader getMenuShader()
{
    jl::Shader shader(
         R"glsl(
            #version 330 core
            layout (location = 0) in vec2 pos;
            layout (location = 1) in vec2 texcoord;
            layout (location = 2) in float elementid;



            out vec2 TexCoord;
            out float elementID;

            uniform float time;

            float gaussian(float x, float mean, float stdDev) {
                float a = 1.0 / (stdDev * sqrt(2.0 * 3.14159265));
                float b = exp(-pow(x - mean, 2.0) / (2.0 * pow(stdDev, 2.0)));
                return a * b;
            }

            void main()
            {
                gl_Position = vec4(pos, 0.0, 1.0);

                if(elementid <= -67.0f && elementid >= -75.0f) {
                    float peak = 1.0 + (abs(elementid) - 67)/2.0;
                    float radius = 0.45;
                    gl_Position.y += gaussian(time*3.0, peak, radius)/3.5;
                }

                if(elementid == -997.0) {
                    gl_Position.x -= time/16.0f;
                }
                if(elementid == -998.0) {
                    gl_Position.x += time/16.0f;
                }

                TexCoord = texcoord;
                elementID = elementid;
            }
        )glsl",
        R"glsl(
            #version 330 core
            out vec4 FragColor;
            in vec2 TexCoord;
            in float elementID;
            uniform sampler2D ourTexture;
            uniform float mousedOverElement;
            uniform float clickedOnElement;
            void main() {
                FragColor = texture(ourTexture, TexCoord);
                if(FragColor.a < 0.1) {
                    discard;
                }
                if(clickedOnElement == elementID) {
                    FragColor = vec4(vec3(1.0, 1.0, 1.0) - FragColor.rgb, 1.0);
                } else if(mousedOverElement == elementID) {
                    FragColor = FragColor + vec4(0.3, 0.3, 0.3, 0.0);
                }
                if(elementID == -99.0f) {
                    FragColor = FragColor - vec4(0.5, 0.5, 0.5, 0.0);
                }
                if(elementID == -97.0f) {
                    discard;
                }
                if(elementID == -98.0f) {
                    FragColor = FragColor - vec4(0.3, 0.3, 0.3, 0.0);
                }
            }
        )glsl",
        "menuShader"

        );
    return shader;
}




void Hud::rebuildDisplayData() {
    float chImageWidth = 35;



    glm::vec2 chLowerLeft(-chImageWidth/windowWidth, -chImageWidth/windowHeight);
    float relHeight = chImageWidth/(windowHeight/2);
    float relWidth = chImageWidth/(windowWidth/2);


    displayData = {
        chLowerLeft.x, chLowerLeft.y,                    0.0f, 0.0f,   -1.0f,
        chLowerLeft.x, chLowerLeft.y+relHeight,          0.0f, 1.0f,   -1.0f,
        chLowerLeft.x+relWidth, chLowerLeft.y+relHeight, 1.0f, 1.0f,   -1.0f,

        chLowerLeft.x+relWidth, chLowerLeft.y+relHeight, 1.0f, 1.0f,   -1.0f,
        chLowerLeft.x+relWidth, chLowerLeft.y,           0.0f, 1.0f,   -1.0f,
        chLowerLeft.x, chLowerLeft.y,                    0.0f, 0.0f,   -1.0f
    };

}

void bindMenuGeometry(GLuint vbo, const float *data, size_t dataSize, GLuint menushad) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(float), data, GL_STATIC_DRAW);

    GLint posAttrib = glGetAttribLocation(menushad, "pos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    GLint texAttrib = glGetAttribLocation(menushad, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    GLint elementIdAttrib = glGetAttribLocation(menushad, "elementid");
    glEnableVertexAttribArray(elementIdAttrib);
    glVertexAttribPointer(elementIdAttrib, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
}

void bindMenuGeometryNoUpload(GLuint vbo, GLuint menushad) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLint posAttrib = glGetAttribLocation(menushad, "pos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    GLint texAttrib = glGetAttribLocation(menushad, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    GLint elementIdAttrib = glGetAttribLocation(menushad, "elementid");
    glEnableVertexAttribArray(elementIdAttrib);
    glVertexAttribPointer(elementIdAttrib, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
}

void Hud::draw()
{
    const bool b = entt::monostate<entt::hashed_string{"activeHand"}>{};
    if(b)
    {
        if(hudVAO == 0)
        {
            glGenVertexArrays(1, &hudVAO);

        }

        static jl::Shader menuShad = getMenuShader();

        static jl::Texture cht("resources/crosshair.png");
        glDisable(GL_CULL_FACE);
        glBindVertexArray(hudVAO);
        glUseProgram(menuShad.shaderID);
        glActiveTexture(GL_TEXTURE6);
        cht.bind_to_unit(6);
        glBindTexture(GL_TEXTURE_2D, cht.id);
        glUniform1i(glGetUniformLocation(menuShad.shaderID, "ourTexture"), 6);

        glUniform1f(glGetUniformLocation(menuShad.shaderID, "mousedOverElement"), 0.0f);

        glUniform1f(glGetUniformLocation(menuShad.shaderID, "clickedOnElement"), 0.0f);
        if(this->uploaded) {
            bindMenuGeometryNoUpload(this->vbo, menuShad.shaderID);
        } else {
            bindMenuGeometry(this->vbo,
            this->displayData.data(),
            this->displayData.size(), menuShad.shaderID);
        }

            glDrawArrays(GL_TRIANGLES, 0, this->displayData.size()/5);


        glEnable(GL_CULL_FACE);
    }
}





