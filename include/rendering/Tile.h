#pragma once

#include "math/Vec2.h"
#include "rendering/Texture.h"

struct Tile
{
    Texture texture;
    Vec2 position;   // World-space position (bottom-left)
    Vec2 size;       // World-space size (width/height in blocks)
};