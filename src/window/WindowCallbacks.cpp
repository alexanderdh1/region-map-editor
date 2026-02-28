#include "window/WindowCallbacks.h"
#include "window/WindowContext.h"
#include "math/BlockCoord.h"
#include <iostream>

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

            // Forward to input system
            context->core->getInput().onMouseButton(
                action == GLFW_PRESS,
                Vec2{ x, y }
            );

            // ---- CLICK TO BLOCK DEBUG ----
            if (action == GLFW_PRESS)
            {
                Vec2 screenPos{ x, y };

                Vec2 worldPos =
                    context->core->getCamera().screenToWorld(screenPos);

                BlockCoord block =
                    context->core->worldToBlock(worldPos);

                std::cout << "Clicked block: X="
                          << block.x
                          << " Z="
                          << block.z
                          << "\n";
            }
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