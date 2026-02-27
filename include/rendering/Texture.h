#pragma once

#include <string>

class Texture
{
public:
    Texture() = default;
    explicit Texture(const std::string& path);

    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void bind() const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    bool isValid() const { return textureId != 0; }

private:
    unsigned int textureId = 0;
    int width = 0;
    int height = 0;
};