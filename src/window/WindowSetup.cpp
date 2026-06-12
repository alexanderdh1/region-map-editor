#include "window/WindowSetup.h"
#include "core/Core.h"

#include <GLFW/glfw3.h>
#include <algorithm>

void setupInitialViewport(GLFWwindow* window, Core& core)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);

    Camera& cam = core.getCamera();
    cam.viewportSize = {
        static_cast<double>(width),
        static_cast<double>(height)
    };

    // Start fully zoomed out: the smallest zoom that still covers the
    // viewport (same limit clampToBounds enforces), centred on the map.
    if (core.getMapWidth() > 0.0 && core.getMapHeight() > 0.0)
    {
        cam.zoom = std::max(
            cam.viewportSize.x / core.getMapWidth(),
            cam.viewportSize.y / core.getMapHeight());
        cam.position = { 0.0, 0.0 };
    }
}