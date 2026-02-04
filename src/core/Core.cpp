#include "core/Core.h"

Core::Core() = default;

void Core::update() {
    if (input.hasPanDelta()) {
        Vec2 delta = input.consumePanDelta();

        // Convert screen-space movement to world-space
        delta.x /= camera.zoom;
        delta.y /= camera.zoom;

        // Invert so dragging feels natural
        camera.panBy({-delta.x, -delta.y});
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
