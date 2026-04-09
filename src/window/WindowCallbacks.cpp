#include "window/WindowCallbacks.h"
#include "window/WindowContext.h"
#include <GLFW/glfw3.h>
#include "math/Vec2.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"

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

    // Mouse button — ImGui first, then UI layer, then map input
    glfwSetMouseButtonCallback(
        window,
        [](GLFWwindow* window, int button, int action, int mods)
        {
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

            // If ImGui wants the mouse, don't forward to game
            if (ImGui::GetIO().WantCaptureMouse) return;

            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
                bool shiftHeld = (mods & GLFW_MOD_SHIFT) != 0;
                ctx->core->getInput().onMouseButton(action == GLFW_PRESS, Vec2{x, y}, shiftHeld);
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
            {
                ctx->core->getInput().onMouseButtonRight(true, Vec2{x, y});
            }
        }
    );

    // Mouse move
    glfwSetCursorPosCallback(
        window,
        [](GLFWwindow* window, double x, double y)
        {
            ImGui_ImplGlfw_CursorPosCallback(window, x, y);

            if (ImGui::GetIO().WantCaptureMouse) return;

            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            ctx->core->getInput().onMouseMove(Vec2{x, y});
        }
    );

    // Scroll
    glfwSetScrollCallback(
        window,
        [](GLFWwindow* window, double xOffset, double yOffset)
        {
            ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);

            if (ImGui::GetIO().WantCaptureMouse) return;

            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            ctx->core->getInput().onScroll(yOffset);
        }
    );

    // Keyboard
    glfwSetKeyCallback(
        window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

            if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));

            // Only block game shortcuts when a text field is actively being edited
            if (ctx->uiLayer->isTextInputActive()) return;

            ctx->uiLayer->onKeyPress(key, *ctx->core);
        }
    );

    // Character input
    glfwSetCharCallback(
        window,
        [](GLFWwindow* window, unsigned int codepoint)
        {
            ImGui_ImplGlfw_CharCallback(window, codepoint);
        }
    );
}