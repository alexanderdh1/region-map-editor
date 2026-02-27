#pragma once

#include <vector>
#include "rendering/Tile.h"

class Camera;

class TileLayer
{
public:
    void addTile(Tile&& tile);

    void render(const Camera& camera);

private:
    std::vector<Tile> tiles;
};