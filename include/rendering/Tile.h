#pragma once

#include "math/Vec2.h"
#include "rendering/Texture.h"

struct Tile
{
    Texture* texture;
    Vec2 position;
    Vec2 size;
};