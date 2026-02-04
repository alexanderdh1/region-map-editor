#include <iostream>
#include "core/Core.h"

int main() {
    Core core;
    Camera& camera = core.getCamera();
    Input& input = core.getInput();

    camera.viewportSize = {800, 600};
    camera.zoom = 1.0;

    // BEFORE any input
    Vec2 before = camera.worldToScreen({0, 0});
    std::cout << "Before drag: "
              << before.x << ", "
              << before.y << "\n";

    // Simulate mouse drag
    input.onMouseButton(true, {100, 100});
    input.onMouseMove({150, 120});
    core.update();

    // AFTER input
    Vec2 after = camera.worldToScreen({0, 0});
    std::cout << "After drag:  "
              << after.x << ", "
              << after.y << "\n";

    return 0;
}
