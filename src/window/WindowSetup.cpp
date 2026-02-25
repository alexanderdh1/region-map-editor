#include "window/WindowSetup.h"
#include "core/Core.h"

#include <GLFW/glfw3.h>

void setupInitialViewport(GLFWwindow* window, Core& core)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);

    core.getCamera().viewportSize = {
        static_cast<double>(width),
        static_cast<double>(height)
    };
}