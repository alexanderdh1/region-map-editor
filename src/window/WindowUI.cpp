#include "window/WindowUI.h"
#include "core/Core.h"
#include "math/Vec2.h"
#include "rendering/Camera.h"

#include <GLFW/glfw3.h>
#include <cmath>

static constexpr double HANDLE_HOVER_PX = 12.0;

void updateWindowTitle(GLFWwindow* window, const Core& core)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    const Camera& camera = core.getCamera();

    // --- Cursor shape ---
    // Switch to a crosshair when hovering near a handle in edit mode,
    // so the user knows they can grab it. Otherwise use the default arrow.
    bool nearHandle = false;

    const EditState& edit = core.getEditState();
    if (edit.isActive() && core.getInput().getDrawTool() == DrawTool::Edit)
    {
        const Region& region = *edit.target;

        auto checkPt = [&](const Vec2& mapPt)
        {
            Vec2 s = camera.worldToScreen(mapPt);
            double dist = std::hypot(mouseX - s.x, mouseY - s.y);
            if (dist <= HANDLE_HOVER_PX) nearHandle = true;
        };

        if (region.geometry.type == GeometryType::Rectangle)
        {
            checkPt({ region.geometry.rectMin.x, region.geometry.rectMax.y }); // TL
            checkPt({ region.geometry.rectMax.x, region.geometry.rectMax.y }); // TR
            checkPt({ region.geometry.rectMax.x, region.geometry.rectMin.y }); // BR
            checkPt({ region.geometry.rectMin.x, region.geometry.rectMin.y }); // BL
        }
        else if (region.geometry.type == GeometryType::Polygon)
        {
            for (const Vec2& p : region.geometry.points)
                checkPt(p);
        }
    }

    // Cursor is created once and reused every frame.
    // glfwTerminate() will destroy it automatically on shutdown.
    static GLFWcursor* crosshair = nullptr;
    if (!crosshair)
        crosshair = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);

    glfwSetCursor(window, nearHandle ? crosshair : nullptr);
}
