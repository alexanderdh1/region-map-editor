#include "core/Core.h"
#include "input/Input.h"
#include "math/Vec2.h"
#include "data/Region.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <memory>

Core::Core() = default;

void Core::update(GLFWwindow* window)
{
    Camera& camera = getCamera();

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    Vec2 mouseScreen{ mouseX, mouseY };

    // PAN
    if (input.hasPanDelta()) {
        Vec2 delta = input.consumePanDelta();
        camera.panBy(delta);
    }

    // ZOOM — zoom toward mouse cursor position
    if (input.hasZoomDelta()) {
        double zoomSteps = input.consumeZoomDelta();
        const double zoomFactor = 1.1;

        Vec2 worldBefore = camera.screenToWorld(mouseScreen);

        if (zoomSteps > 0)
            camera.zoomBy(zoomFactor);
        else
            camera.zoomBy(1.0 / zoomFactor);

        Vec2 worldAfter = camera.screenToWorld(mouseScreen);

        camera.position.x += (worldBefore.x - worldAfter.x);
        camera.position.y += (worldBefore.y - worldAfter.y);
    }

    // Enforce zoom limits and keep camera within world bounds
    camera.clampToBounds(worldWidth, worldHeight);

    // RECT DRAWING — finalise a rectangle when the user releases
    if (input.hasCompletedRect())
    {
        Vec2 screenA = input.getDrawStart();
        Vec2 screenB = input.getDrawCurrent();

        Vec2 worldA = camera.screenToWorld(screenA);
        Vec2 worldB = camera.screenToWorld(screenB);

        // Normalise so min < max regardless of drag direction
        RegionGeometry geom;
        geom.type    = GeometryType::Rectangle;
        geom.rectMin = { std::min(worldA.x, worldB.x),
                         std::min(worldA.y, worldB.y) };
        geom.rectMax = { std::max(worldA.x, worldB.x),
                         std::max(worldA.y, worldB.y) };

        // Ignore tiny accidental clicks
        if (geom.isValid())
        {
            auto region      = std::make_unique<Region>();
            region->id       = regionTree.nextId();
            region->geometry = geom;
            regionTree.addRegion(std::move(region));
        }

        input.consumeCompletedRect();
    }

    // CLICK — hit-test regions, select or deselect
    if (input.hasClick())
    {
        Vec2 screenPos = input.consumeClick();
        Vec2 worldPos  = camera.screenToWorld(screenPos);

        // Find topmost region under cursor (last match = drawn on top)
        Region* hit = nullptr;
        regionTree.forEach([&](Region& r)
        {
            if (r.geometry.contains(worldPos))
                hit = &r;
        });

        selection.select(hit); // nullptr = deselect
    }
}

Camera& Core::getCamera() {
    return camera;
}

const Camera& Core::getCamera() const {
    return camera;
}

Input& Core::getInput() {
    return input;
}

const Input& Core::getInput() const {
    return input;
}

void Core::setWorldSize(double width, double height)
{
    worldWidth = width;
    worldHeight = height;



    double minZoomX = camera.viewportSize.x / worldWidth;
    double minZoomY = camera.viewportSize.y / worldHeight;

    camera.minZoom = std::max(minZoomX, minZoomY);
    
}

void Core::setWorldBlockBounds(
    int minBlockX,
    int minBlockZ,
    int maxBlockX,
    int maxBlockZ
)
{
    worldMinBlockX = minBlockX;
    worldMinBlockZ = minBlockZ;
    worldMaxBlockX = maxBlockX;
    worldMaxBlockZ = maxBlockZ;
}

Vec2 Core::blockToWorld(const BlockCoord& block) const
{
    double worldX = block.x - worldMinBlockX;

    // Flip Z
    double worldZ = worldMaxBlockZ - block.z;

    worldX -= worldWidth  / 2.0;
    worldZ -= worldHeight / 2.0;

    return { worldX, worldZ };
}

BlockCoord Core::worldToBlock(const Vec2& worldPos) const
{
    double localX = worldPos.x + worldWidth  / 2.0;
    double localZ = worldPos.y + worldHeight / 2.0;

    int blockX = static_cast<int>(localX) + worldMinBlockX;

    // Flip Z
    int blockZ = worldMaxBlockZ - static_cast<int>(localZ);

    return { blockX, blockZ };
}