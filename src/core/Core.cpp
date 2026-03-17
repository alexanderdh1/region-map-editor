#include "core/Core.h"
#include "input/Input.h"
#include "math/Vec2.h"
#include <GLFW/glfw3.h>
#include <algorithm>

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