#include "textureface.h"
#include "world/WorldRenderer.h"


TextureFace::TextureFace(
    glm::vec2 tl,
    glm::vec2 bl,
    glm::vec2 br,
    glm::vec2 tr
) : tl(tl), bl(bl), br(br), tr(tr) {

}

TextureFace::TextureFace(
    int x,
    int y
) :
    bl(glm::vec2(0.0f + onePixel + (texSlotWidth * (float)x), 1.0f - ((float)y * texSlotWidth) - onePixel)),
    tl(glm::vec2(0.0f + onePixel + (texSlotWidth * (float)x), 1.0f - ((float)y * texSlotWidth) - textureWidth - onePixel)),
    tr(glm::vec2(0.0f + onePixel + (texSlotWidth * (float)x) + textureWidth, 1.0f - ((float)y * texSlotWidth) - textureWidth - onePixel)),
    br(glm::vec2(0.0f + onePixel + (texSlotWidth * (float)x) + textureWidth, 1.0f - ((float)y * texSlotWidth) - onePixel))
{

}