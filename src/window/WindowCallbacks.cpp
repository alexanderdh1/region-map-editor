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

            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            ctx->core->getCamera().viewportSize = { (double)width, (double)height };
        }
    );

    // Mouse button — forward to Input only
    glfwSetMouseButtonCallback(
        window,
        [](GLFWwindow* window, int button, int action, int mods)
        {
            if (button != GLFW_MOUSE_BUTTON_LEFT) return;

            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            bool shiftHeld = (mods & GLFW_MOD_SHIFT) != 0;
            ctx->core->getInput().onMouseButton(action == GLFW_PRESS, Vec2{x, y}, shiftHeld);
        }
    );

    // Mouse move — forward to Input only
    glfwSetCursorPosCallback(
        window,
        [](GLFWwindow* window, double x, double y)
        {
            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            ctx->core->getInput().onMouseMove(Vec2{x, y});
        }
    );

    // Scroll — forward to Input only
    glfwSetScrollCallback(
        window,
        [](GLFWwindow* window, double /*xOffset*/, double yOffset)
        {
            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            ctx->core->getInput().onScroll(yOffset);
        }
    );

    // Keyboard — forward to UILayer, which owns all UI key handling
    glfwSetKeyCallback(
        window,
        [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
        {
            if (action != GLFW_PRESS) return;
            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            ctx->uiLayer->onKeyPress(key, *ctx->core);
        }
    );
}