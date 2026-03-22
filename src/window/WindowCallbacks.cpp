#include "window/WindowCallbacks.h"
#include "window/WindowContext.h"
#include "data/RegionStatus.h"
#include <GLFW/glfw3.h>
#include "math/Vec2.h"

// Preset colours cycled with [C]
static const float colourPresets[][4] = {
    { 0.40f, 0.60f, 1.00f, 0.35f }, // blue   (default)
    { 0.30f, 0.85f, 0.45f, 0.35f }, // green
    { 0.95f, 0.35f, 0.35f, 0.35f }, // red
    { 0.95f, 0.75f, 0.20f, 0.35f }, // yellow
    { 0.75f, 0.35f, 0.95f, 0.35f }, // purple
    { 0.95f, 0.55f, 0.20f, 0.35f }, // orange
};
static const int numColours = 6;

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
        [](GLFWwindow* window, int button, int action, int mods)
        {
            auto* context =
                static_cast<WindowContext*>(glfwGetWindowUserPointer(window));

            if (button != GLFW_MOUSE_BUTTON_LEFT)
                return;

            double x, y;
            glfwGetCursorPos(window, &x, &y);

            bool shiftHeld = (mods & GLFW_MOD_SHIFT) != 0;

            context->core->getInput().onMouseButton(
                action == GLFW_PRESS,
                Vec2{ x, y },
                shiftHeld
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

    // Keyboard
    glfwSetKeyCallback(
        window,
        [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
        {
            if (action != GLFW_PRESS) return;

            auto* context =
                static_cast<WindowContext*>(glfwGetWindowUserPointer(window));

            auto& input     = context->core->getInput();
            auto& selection = context->core->getSelection();
            Region* r       = selection.selectedRegion;

            // --- Tool switching (always available) ---
            if (key == GLFW_KEY_R)
            {
                input.setDrawTool(DrawTool::Rectangle);
                input.cancelPolygon();
                return;
            }
            if (key == GLFW_KEY_P)
            {
                input.setDrawTool(DrawTool::Polygon);
                return;
            }

            // --- Escape: cancel drawing or close popup ---
            if (key == GLFW_KEY_ESCAPE)
            {
                if (input.isDrawingPolygon())
                    input.cancelPolygon();
                else if (input.isDrawingRect())
                    input.cancelRect();
                else
                    selection.clear();
                return;
            }

            // --- Region editing (only when popup is open) ---
            if (!r) return;

            switch (key)
            {
                case GLFW_KEY_1:
                    r->status = RegionStatus::None; break;
                case GLFW_KEY_2:
                    r->status = RegionStatus::InProgress; break;
                case GLFW_KEY_3:
                    r->status = RegionStatus::Done; break;

                case GLFW_KEY_C:
                {
                    int next = 0;
                    for (int i = 0; i < numColours; i++)
                    {
                        if (colourPresets[i][0] == r->colorR &&
                            colourPresets[i][1] == r->colorG)
                        {
                            next = (i + 1) % numColours;
                            break;
                        }
                    }
                    r->colorR = colourPresets[next][0];
                    r->colorG = colourPresets[next][1];
                    r->colorB = colourPresets[next][2];
                    r->colorA = colourPresets[next][3];
                    break;
                }

                case GLFW_KEY_DELETE:
                {
                    RegionId id = r->id;
                    selection.clear();
                    context->core->getRegionTree().removeRegion(id);
                    break;
                }

                default: break;
            }
        }
    );
}