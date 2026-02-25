#include "core/Core.h"
#include "input/Input.h"
#include "math/Vec2.h"
#include <GLFW/glfw3.h>

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
