#ifndef TEXTURE2DARRAY_H
#define TEXTURE2DARRAY_H

#include "PrecompHeader.h"


namespace jl {

    class Texture2DArray {
    public:
        GLuint textureID; // OpenGL texture ID
        int width, height, depth; // Dimensions of the 3D texture

        // Constructor: Takes a list of file paths to load as slices
        explicit Texture2DArray(const std::vector<std::string>& paths);

        // Destructor
        ~Texture2DArray();

        // Bind the 3D texture
        void bind(GLuint unit = 0) const;

    private:
        // Helper function to load a single 2D texture slice
        static unsigned char* loadTextureData(const std::string& path, int* width, int* height);
    };

}

#endif // TEXTURE2DARRAY_H