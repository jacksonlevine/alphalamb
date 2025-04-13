#include "Texture2DArray.h"



namespace jl {

// Constructor
Texture2DArray::Texture2DArray(const std::vector<std::string>& paths) {
    depth = static_cast<int>(paths.size());

    if (depth == 0) {
        std::cerr << "Error: No texture paths provided!" << std::endl;
        return;
    }

    // Load the first texture to determine width and height
    int channels;
    unsigned char* data = loadTextureData(paths[0], &width, &height);
    if (!data) {
        std::cerr << "Error: Failed to load texture: " << paths[0] << std::endl;
        return;
    }

    // Generate and bind the 3D texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

    if(width < 1)
    {

    }
    if(height < 1)
    {

    }
    if(depth < 1)
    {


    }



    // Allocate storage for the 3D texture
    //glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, width, height, depth);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, depth);

    if(width < 0)
    {

    }
    if(height < 0)
    {

    }

    // Upload the first texture slice
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    // Load and upload the remaining slices
    for (int i = 1; i < depth; ++i) {
        data = loadTextureData(paths[i], &width, &height);
        if (!data) {
            std::cerr << "Error: Failed to load texture: " << paths[i] << std::endl;
            continue;
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    glBindTexture(GL_TEXTURE_2D_ARRAY, 0); // Unbind the texture
}

// Destructor
Texture2DArray::~Texture2DArray() {
    glDeleteTextures(1, &textureID);
}

// Bind the texture to a texture unit
void Texture2DArray::bind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
}

// Helper function to load a single 2D texture
unsigned char* Texture2DArray::loadTextureData(const std::string& path, int* width, int* height) {
    //stbi_set_flip_vertically_on_load(true); // Flip vertically if needed
    int channels;
    unsigned char* data = stbi_load(path.c_str(), width, height, &channels, STBI_rgb_alpha);
    if (!data) {
        std::cerr << "Error: Failed to load image: " << path << std::endl;
    }
    return data;
}

}
