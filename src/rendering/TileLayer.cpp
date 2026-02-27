#include "rendering/TileLayer.h"
#include "rendering/Camera.h"

#include <GLFW/glfw3.h>
#include <iostream>

void TileLayer::addTile(Tile&& tile)
{
    tiles.emplace_back(std::move(tile));
}

void TileLayer::render(const Camera& camera)
{
    glEnable(GL_TEXTURE_2D);

    for (const Tile& tile : tiles)
    {
        if (!tile.texture.isValid())
            continue;

        tile.texture.bind();

        // World corners
        Vec2 bottomLeft  = tile.position;
        Vec2 bottomRight = { tile.position.x + tile.size.x, tile.position.y };
        Vec2 topRight    = { tile.position.x + tile.size.x, tile.position.y + tile.size.y };
        Vec2 topLeft     = { tile.position.x, tile.position.y + tile.size.y };

        // Convert to screen-space
        Vec2 bl = camera.worldToScreen(bottomLeft);
        Vec2 br = camera.worldToScreen(bottomRight);
        Vec2 tr = camera.worldToScreen(topRight);
        Vec2 tl = camera.worldToScreen(topLeft);

        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 0.0f); glVertex2d(bl.x, bl.y);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(br.x, br.y);
        glTexCoord2f(1.0f, 1.0f); glVertex2d(tr.x, tr.y);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(tl.x, tl.y);

        glEnd();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}