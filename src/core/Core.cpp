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

    // ZOOM
    if (input.hasZoomDelta()) {

        double zoomSteps = input.consumeZoomDelta();
        const double zoomFactor = 1.1;

        Vec2 worldBefore = camera.screenToWorld(mouseScreen);

        if (zoomSteps > 0)
            camera.zoomBy(zoomFactor);
        else if (zoomSteps < 0)
            camera.zoomBy(1.0 / zoomFactor);

        Vec2 worldAfter = camera.screenToWorld(mouseScreen);

        camera.position.x += (worldBefore.x - worldAfter.x);
        camera.position.y += (worldBefore.y - worldAfter.y);
    }
    // ---- Dynamic minimum zoom ----
    double minZoomX = camera.viewportSize.x / worldWidth;
    double minZoomY = camera.viewportSize.y / worldHeight;

    // Choose the stricter one
    double dynamicMinZoom = std::max(minZoomX, minZoomY);

    // Enforce zoom clamp
    if (camera.zoom < dynamicMinZoom)
    {
        camera.zoom = dynamicMinZoom;
    }
    // ---- Hard clamp camera ----

    // World bounds (single image case)
    double halfW = worldWidth / 2.0;
    double halfH = worldHeight / 2.0;

    double worldMinX = -halfW;
    double worldMaxX =  halfW;
    double worldMinY = -halfH;
    double worldMaxY =  halfH;

    // Visible area
    double visibleHalfW = camera.viewportSize.x / (2.0 * camera.zoom);
    double visibleHalfH = camera.viewportSize.y / (2.0 * camera.zoom);

    // X axis
    double minCamX = worldMinX + visibleHalfW;
    double maxCamX = worldMaxX - visibleHalfW;

    // If zoomed out more than world size → center
    if (visibleHalfW >= halfW)
    {
        camera.position.x = 0.0;
    }
    else
    {
        if (camera.position.x < minCamX)
            camera.position.x = minCamX;
        if (camera.position.x > maxCamX)
            camera.position.x = maxCamX;
    }

    // Y axis
    double minCamY = worldMinY + visibleHalfH;
    double maxCamY = worldMaxY - visibleHalfH;

    if (visibleHalfH >= halfH)
    {
        camera.position.y = 0.0;
    }
    else
    {
        if (camera.position.y < minCamY)
            camera.position.y = minCamY;
        if (camera.position.y > maxCamY)
            camera.position.y = maxCamY;
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