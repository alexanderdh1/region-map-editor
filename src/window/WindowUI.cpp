#include "window/WindowUI.h"
#include "core/Core.h"
#include "math/Vec2.h"
#include "rendering/Camera.h"

#include <GLFW/glfw3.h>
#include <sstream>
#include <iomanip>

void updateWindowTitle(GLFWwindow* window, const Core& core)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    const Camera& camera = core.getCamera();

    Vec2 mouseWorld =
        camera.screenToWorld({ mouseX, mouseY });

    std::ostringstream title;
    title << "Spatial Map Editor | "
          << "Cam: ("
          << std::fixed << std::setprecision(2)
          << camera.position.x << ", "
          << camera.position.y
          << ") | Cursor: ("
          << mouseWorld.x << ", "
          << mouseWorld.y
          << ") | Zoom: "
          << camera.zoom;

    glfwSetWindowTitle(window, title.str().c_str());
}