#include "window/WindowCallbacks.h"
#include "window/WindowContext.h"

#include <GLFW/glfw3.h>
#include "math/Vec2.h"

void setupWindowCallbacks(GLFWwindow* window, WindowContext* context)
{
    glfwSetWindowUserPointer(window, context);

    // Resize
    glfwSetFramebufferSizeCallback(
        window,
        [](GLFWwindow* window, int width, int height)
        {
            glViewport(0, 0, width, height);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, width, height, 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);

            auto* context =
                static_cast<WindowContext*>(glfwGetWindowUserPointer(window));

            context->core->getCamera().viewportSize =
                { (double)width, (double)height };
        }
    );

    // Mouse button
    glfwSetMouseButtonCallback(
        window,
        [](GLFWwindow* window, int button, int action, int /*mods*/)
        {
            auto* context =
                static_cast<WindowContext*>(glfwGetWindowUserPointer(window));

            if (button != GLFW_MOUSE_BUTTON_LEFT)
                return;

            double x, y;
            glfwGetCursorPos(window, &x, &y);

            context->core->getInput().onMouseButton(
                action == GLFW_PRESS,
                Vec2{ x, y }
            );
        }
    );

    // Mouse move
    glfwSetCursorPosCallback(
        window,
        [](GLFWwindow* window, double x, double y)
        {
            auto* context =
                static_cast<WindowContext*>(glfwGetWindowUserPointer(window));

            context->core->getInput().onMouseMove(Vec2{ x, y });
        }
    );

    // Scroll
    glfwSetScrollCallback(
        window,
        [](GLFWwindow* window, double /*xOffset*/, double yOffset)
        {
            auto* context =
                static_cast<WindowContext*>(glfwGetWindowUserPointer(window));

            context->core->getInput().onScroll(yOffset);
        }
    );
}