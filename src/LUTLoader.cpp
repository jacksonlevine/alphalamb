//
// Created by jack on 3/23/2025.
//
#include "LUTLoader.h"



GLuint load3DLUT(const char* filename)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);

    if (!image) {
        std::cerr << "Failed to load LUT image: " << filename << std::endl;
        return 0;
    }

    if (width % 8 != 0 || height % 8 != 0) {
        std::cerr << "ERROR: LUT image size must be a multiple of 8!\n";
        stbi_image_free(image);
        return 0;
    }

    int lutSize = width / 8;
    std::vector<unsigned char> lutData(lutSize * lutSize * lutSize * channels);

    for (int by = 0; by < 8; by++) {
        for (int bx = 0; bx < 8; bx++) {
            int z = by * 8 + bx;

            for (int y = 0; y < lutSize; y++) {
                for (int x = 0; x < lutSize; x++) {
                    int srcX = bx * lutSize + x;
                    int srcY = by * lutSize + y;

                    int srcIdx = (srcY * width + srcX) * channels;
                    int dstIdx = (z * lutSize * lutSize + y * lutSize + x) * channels;

                    for (int c = 0; c < channels; c++) {
                        lutData[dstIdx + c] = image[srcIdx + c];
                    }
                }
            }
        }
    }

    stbi_image_free(image);

    GLuint lutTexture;
    glGenTextures(1, &lutTexture);
    glBindTexture(GL_TEXTURE_3D, lutTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, (channels == 4) ? GL_RGBA8 : GL_RGB8,
                 lutSize, lutSize, lutSize, 0, (channels == 4) ? GL_RGBA : GL_RGB,
                 GL_UNSIGNED_BYTE, lutData.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return lutTexture;
}
