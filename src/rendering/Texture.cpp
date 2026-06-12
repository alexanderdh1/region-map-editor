#include "rendering/Texture.h"

#include <GLFW/glfw3.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Windows ships GL 1.1 headers; this constant is GL 1.4
#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif

Texture::Texture(const std::string& path)
{
    stbi_set_flip_vertically_on_load(true);

    int channels;

    unsigned char* data =
        stbi_load(path.c_str(), &width, &height, &channels, 4);

    if (!data) {
        std::cerr << "Failed to load image: " << path << "\n";
        return;
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Fixed-function mipmap generation (GL 1.4) — removes shimmering when zoomed out
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
}

Texture::~Texture()
{
    if (textureId != 0) {
        glDeleteTextures(1, &textureId);
    }
}

Texture::Texture(Texture&& other) noexcept
{
    textureId = other.textureId;
    width = other.width;
    height = other.height;

    other.textureId = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        if (textureId != 0) {
            glDeleteTextures(1, &textureId);
        }

        textureId = other.textureId;
        width = other.width;
        height = other.height;

        other.textureId = 0;
    }

    return *this;
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, textureId);
}