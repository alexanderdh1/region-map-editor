#pragma once

#include <memory>
#include "math/Vec2.h"
#include "rendering/Texture.h"

struct Tile
{
    std::shared_ptr<Texture> texture;
    Vec2 position;
    Vec2 size;
};