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
    glColor3f(1.0f, 1.0f, 1.0f);

    // ---- Calculate visible world bounds ----
    Vec2 worldA = camera.screenToWorld({0.0, 0.0});
    Vec2 worldB = camera.screenToWorld(camera.viewportSize);

    double minX = std::min(worldA.x, worldB.x);
    double maxX = std::max(worldA.x, worldB.x);
    double minY = std::min(worldA.y, worldB.y);
    double maxY = std::max(worldA.y, worldB.y);

    for (const Tile& tile : tiles)
    {
        // ---- Tile bounds ----
        double tileMinX = tile.position.x;
        double tileMaxX = tile.position.x + tile.size.x;
        double tileMinY = tile.position.y;
        double tileMaxY = tile.position.y + tile.size.y;

        // ---- AABB overlap test ----
        bool visible =
            tileMaxX >= minX &&
            tileMinX <= maxX &&
            tileMaxY >= minY &&
            tileMinY <= maxY;

        if (!visible)
            continue;

        tile.texture->bind();

        Vec2 bl = camera.worldToScreen({tileMinX, tileMinY});
        Vec2 br = camera.worldToScreen({tileMaxX, tileMinY});
        Vec2 tr = camera.worldToScreen({tileMaxX, tileMaxY});
        Vec2 tl = camera.worldToScreen({tileMinX, tileMaxY});

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(bl.x, bl.y);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(br.x, br.y);
        glTexCoord2f(1.0f, 1.0f); glVertex2d(tr.x, tr.y);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(tl.x, tl.y);

        glEnd();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}