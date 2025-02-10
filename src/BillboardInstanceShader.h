//
// Created by jack on 10/13/2024.
//

#ifndef BILLBOARDINSTANCESHADER_H
#define BILLBOARDINSTANCESHADER_H
#include "Shader.h"

extern GLuint billboardVAO;
extern GLuint billboardVBO;


extern GLuint billboardInstancesVBO;
extern GLuint billboardAnimationStatesVBO;

constexpr size_t MAX_BILLBOARDS = 100;


extern float BILLBOARDWIDTH;

struct Billboard
{
    glm::vec3 position = glm::vec3(0.0);
    glm::vec3 direction = glm::vec3(0.0,1.0,0.0);
    float characterNum = 0.0f;
    float hidden = 0.0f;
};

struct AnimationState
{
    float actionNum = 0.0f;
    float timestarted = 0.0f;
    float timescale = 1.0f;
};


enum AnimationName
{
    IDLE,
    RUN,
    JUMP,
    LEFTSTRAFE,
    RIGHTSTRAFE,
    BACKWARDSRUN
};

jl::Shader getBillboardInstanceShader();

namespace jl
{



inline void prepareBillboard()
{
    glGenVertexArrays(1, &billboardVAO);
    glBindVertexArray(billboardVAO);
    glGenBuffers(1, &billboardVBO);

    glGenBuffers(1, &billboardInstancesVBO);
    glGenBuffers(1, &billboardAnimationStatesVBO);

    glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);



    const glm::vec3 lowerleft = glm::vec3(0.0)  - (glm::vec3(1.0, 0.0, 0.0) * (BILLBOARDWIDTH / 2.0f));
    const glm::vec3 upperright = glm::vec3(0.0) + (glm::vec3(0.0, 1.0, 0.0) * (BILLBOARDWIDTH)) + (glm::vec3(1.0, 0.0, 0.0) * (BILLBOARDWIDTH / 2.0f));


    const float vertdata[] = {
        lowerleft.x , lowerleft.y, lowerleft.z, 0.0f, 1.0f,
        lowerleft.x , upperright.y, lowerleft.z , 1.0f, 5.0f,
        upperright.x , upperright.y , lowerleft.z, 2.0f, 5.0f,

        upperright.x , upperright.y , lowerleft.z, 2.0f, 5.0f,
        upperright.x  , lowerleft.y , lowerleft.z ,  3.0f, 5.0f,
        lowerleft.x , lowerleft.y, lowerleft.z, 0.0f, 5.0f,
    };

    glBufferData(GL_ARRAY_BUFFER, std::size(vertdata) * sizeof(float),
            vertdata, GL_STATIC_DRAW
        );


    //VERTEX POSITION   vec3
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);


    //CORNER ID   float
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

//BLOCK ID     float
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);

}

inline void updateAnimStates(std::vector<AnimationState>& instanceanimstates)
{
        glBindVertexArray(billboardVAO);
        glBindBuffer(GL_ARRAY_BUFFER, billboardAnimationStatesVBO);


        glBufferData(GL_ARRAY_BUFFER, std::size(instanceanimstates) * sizeof(AnimationState),
                instanceanimstates.data(), GL_STATIC_DRAW
            );

        //instance animation action num
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE,
            sizeof(AnimationState), (void*)0);
        glEnableVertexAttribArray(6);
        glVertexAttribDivisor(6, 1);

        //instance animation start time
        glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE,
                sizeof(AnimationState), (void*)sizeof(float));
        glEnableVertexAttribArray(7);
        glVertexAttribDivisor(7, 1);

        //instance animation time scale
        glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE,
                sizeof(AnimationState), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(8);
        glVertexAttribDivisor(8, 1);

}

inline void updateBillboards(std::vector<Billboard>& instancepositions)
{
    glBindVertexArray(billboardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, billboardInstancesVBO);


    glBufferData(GL_ARRAY_BUFFER, std::size(instancepositions) * sizeof(Billboard),
            instancepositions.data(), GL_STATIC_DRAW
        );



    //INSTANCE POSITION   vec3
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
        sizeof(Billboard), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    //INSTANCE DIRECTION
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE,
    sizeof(Billboard), (void*)(sizeof(glm::vec3)));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

    //INSTANCE CHARACTER NUMBER
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE,
    sizeof(Billboard), (void*)(2*sizeof(glm::vec3)));
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);

    //INSTANCE HIDDEN
    glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE,
    sizeof(Billboard), (void*)(2*sizeof(glm::vec3) + sizeof(float)));
    glEnableVertexAttribArray(9);
    glVertexAttribDivisor(9, 1);

}

inline void drawBillboards(const int num)
{
    glBindVertexArray(billboardVAO);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, num);
}

inline void drawBillboard()
{
    glBindVertexArray(billboardVAO);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 4);
}

inline void bindBillboardGeometry(GLuint vbo, std::vector<float> &)
{

}





}
inline jl::Shader getBillboardInstanceShader()
{
    jl::Shader shader(
        R"glsl(
            #version 330 core

            layout(location = 0) in vec3 vertexPosition; // Quad vertex positions
            layout(location = 1) in float cornerID;    // Corner ID of quad
            layout(location = 2) in float blockID;


            layout(location = 3) in vec3 instancePosition;
            layout(location = 4) in vec3 instanceDirection;
            layout(location = 5) in float characterNum;
            layout(location = 6) in float animationIndex;
            layout(location = 7) in float startTime;
            layout(location = 8) in float timeScale;
            layout(location = 9) in float hidden;

            out vec2 tcoord;
            out float animIndex;
            out float hide;

            uniform mat4 mvp;
            uniform vec3 camPos;
            uniform float time;

            void main() {
                hide = hidden;
                animIndex = animationIndex;

                vec3 realPosition = instancePosition;

                // Calculate the billboard orientation, restricting rotation to Y-axis only
                vec3 look = normalize(realPosition - camPos);
                //look.y = 0.0; // Lock Y-axis rotation
                vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), look)); // Right vector
                vec3 up = vec3(0.0, 1.0, 0.0); // Fixed up vector in world space

                // Compute final position by applying right and up offsets in world space
                vec3 billboardedPosition = realPosition + (vertexPosition.x * right + vertexPosition.y * up);

                // Transform to clip space
                gl_Position = mvp * vec4(billboardedPosition + vec3(0.0, -2.0, 0.0), 1.0);

                // UV calculation and other code remains the same
                vec2 baseUV = vec2(mod(blockID, 16.0f) * 0.03308823529411764705882352941176f, 1.0f - floor((blockID/16.0f) * 0.52941176470588235294117647058824f));

                float animationFrame = floor(mod((time-startTime) * (30.0 * timeScale), 16.0f));

                vec3 dirToCam = normalize(camPos - instancePosition);
                float dotProd = dot(dirToCam, instanceDirection*-1.0);
                vec3 crossProd = cross(instanceDirection*-1.0, dirToCam);
                float angle = atan(crossProd.y, dotProd);

                float normAngle = (angle + 3.1415926535897932384626433832795) / (2.0 * 3.1415926535897932384626433832795);



                float offset = min(round(normAngle * 16.0), 15.0);


                baseUV = vec2((characterNum * 0.25) + (animationFrame * (1.0 / 64.0)), offset / 16.0);




                // Selecting UV based on cornerID
                if (cornerID == 0.0) {
                    tcoord = vec2(baseUV.x + (1.0 / 64.0), baseUV.y  + (1.0 / 16.0));
                } else if (cornerID == 1.0) {
                    tcoord = vec2(baseUV.x + (1.0 / 64.0), baseUV.y);
                } else if (cornerID == 2.0) {
                    tcoord = vec2(baseUV.x, baseUV.y);
                } else if (cornerID == 3.0) {
                    tcoord = baseUV + vec2(0.0, (1.0 / 16.0));
                }
            }
        )glsl",
        R"glsl(
            #version 330 core
            in vec2 tcoord;
            in float animIndex;
            in float hide;
            out vec4 FragColor;
            uniform sampler2DArray ourTexture;

            void main()
            {
                vec4 texColor = texture(ourTexture, vec3(tcoord, animIndex));
                FragColor = texColor;
                if(texColor.a < 0.1 || hide > 0.2f) {
                    discard;
                }
            }

        )glsl",
        "billboardInstanceShader"

        );
    return shader;
}


#endif //BILLBOARDINSTANCESHADER_H
