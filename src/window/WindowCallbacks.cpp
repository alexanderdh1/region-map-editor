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

    // Mouse button — UI gets first chance, then forward to Input
    glfwSetMouseButtonCallback(
        window,
        [](GLFWwindow* window, int button, int action, int mods)
        {
            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
                bool shiftHeld = (mods & GLFW_MOD_SHIFT) != 0;

                // Track whether UI consumed the press so we can block the release too
                static bool uiConsumedPress = false;

                if (action == GLFW_PRESS)
                {
                    if (!shiftHeld)
                    {
                        uiConsumedPress = ctx->uiLayer->onMouseClick(Vec2{x, y}, *ctx->core);
                        if (uiConsumedPress) return;
                    }
                    else
                    {
                        uiConsumedPress = false;
                    }
                }
                else if (action == GLFW_RELEASE)
                {
                    if (uiConsumedPress)
                    {
                        uiConsumedPress = false;
                        return; // block release so Input never sees it
                    }
                }

                ctx->core->getInput().onMouseButton(action == GLFW_PRESS, Vec2{x, y}, shiftHeld);
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
            {
                ctx->core->getInput().onMouseButtonRight(true, Vec2{x, y});
            }
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
            if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            ctx->uiLayer->onKeyPress(key, *ctx->core);
        }
    );

    // Character input — for text fields (handles keyboard layout correctly)
    glfwSetCharCallback(
        window,
        [](GLFWwindow* window, unsigned int codepoint)
        {
            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            ctx->uiLayer->onCharInput(codepoint, *ctx->core);
        }
    );
}