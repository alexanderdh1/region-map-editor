#include "input/Input.h"
#include <cmath>
#include <GLFW/glfw3.h>

static constexpr double DOUBLE_CLICK_SECONDS = 0.35;
static constexpr double DOUBLE_CLICK_RADIUS  = 10.0;
static constexpr double EDIT_DRAG_THRESHOLD  = 3.0;

void Input::onMouseButton(bool pressed, const Vec2& mousePos, bool shiftHeld)
{
    if (pressed)
    {
        // ---- EDIT MODE ----
        if (activeTool == DrawTool::Edit)
        {
            // We don't know yet if the press is on a handle (that check lives in
            // Core), so we always arm the edit state; Core cancels it on a miss.
            mode                = InputMode::Edit;
            editMouseButtonHeld = true;
            editDragging        = false;
            editDidDrag         = false;
            editDragStartPos    = mousePos;
            editLastMousePos    = mousePos;
            editDragDelta       = { 0.0, 0.0 };
            editDragTotalDelta  = { 0.0, 0.0 };
            return;
        }

        // ---- POLYGON MODE ----
        if (shiftHeld && activeTool == DrawTool::Polygon)
        {
            mode = InputMode::DrawPolygon;

            double now  = glfwGetTime();
            double dist = std::hypot(
                mousePos.x - lastClickPos.x,
                mousePos.y - lastClickPos.y
            );
            bool isDoubleClick =
                (now - lastClickTime < DOUBLE_CLICK_SECONDS) &&
                (dist < DOUBLE_CLICK_RADIUS);

            if (isDoubleClick && polyDrawing && polyMapPoints.size() >= 3)
            {
                completedPoly    = polyMapPoints;
                polyMapPoints.clear();
                pendingPolyPoint = false;
                polyDrawing      = false;
                polyCompleted    = true;
            }
            else
            {
                polyDrawing      = true;
                pendingPolyPoint = true;
                pendingPolyPos   = mousePos;
            }

            lastClickTime = now;
            lastClickPos  = mousePos;
            return;
        }

        // ---- RECT MODE ----
        if (shiftHeld && activeTool == DrawTool::Rectangle)
        {
            mode            = InputMode::DrawRect;
            drawing         = true;
            rectCompleted   = false;
            rectJustStarted = true;
            drawStart       = mousePos;
            drawCurrent     = mousePos;
            return;
        }

        // ---- NAVIGATE ----
        mode         = InputMode::Navigate;
        dragging     = true;
        didDrag      = false;
        lastMousePos = mousePos;
        clickPos     = mousePos;
    }
    else // released
    {
        // Edit mode release
        if (activeTool == DrawTool::Edit)
        {
            editMouseButtonHeld = false;

            if (editDragging)
            {
                editDragging       = false;
                editDragEndPending = true;
            }
            else if (!editDidDrag)
            {
                editClickPending = true;
                editClickPos     = mousePos;
            }
            editDidDrag        = false;
            editDragDelta      = { 0.0, 0.0 };
            editDragTotalDelta = { 0.0, 0.0 };
            dragging = false;
            didDrag  = false;
            mode     = InputMode::Navigate;
            return;
        }

        if (mode == InputMode::DrawRect && drawing)
        {
            drawing       = false;
            rectCompleted = true;
            mode          = InputMode::Navigate;
            return;
        }

        if (mode == InputMode::DrawPolygon)
            return;

        if (!didDrag)
        {
            double now  = glfwGetTime();
            double dist = std::hypot(
                clickPos.x - navLastClickPos.x,
                clickPos.y - navLastClickPos.y
            );

            if (now - navLastClickTime < DOUBLE_CLICK_SECONDS &&
                dist < DOUBLE_CLICK_RADIUS)
            {
                doubleClickPending = true;
                doubleClickPos     = clickPos;
                navLastClickTime   = 0.0; // a triple-click is not two doubles
            }
            else
            {
                clickPending     = true;
                navLastClickTime = now;
                navLastClickPos  = clickPos;
            }
        }

        dragging = false;
        didDrag  = false;
        mode     = InputMode::Navigate;
    }
}

void Input::onMouseButtonMiddle(bool pressed, const Vec2& mousePos)
{
    panDragging = pressed;
    panLastPos  = mousePos;
}

void Input::onMouseMove(const Vec2& mousePos)
{
    drawCurrent = mousePos;

    // Middle-mouse pan has priority and works in every mode
    if (panDragging)
    {
        panDelta.x += mousePos.x - panLastPos.x;
        panDelta.y += mousePos.y - panLastPos.y;
        panLastPos  = mousePos;
        return;
    }

    if (activeTool == DrawTool::Edit)
    {
        // Only accumulate drag state while the mouse button is actually held.
        // Without this guard, releasing the button mid-motion left editDragging=true
        // and caused the "stuck mouse" bug.
        if (editMouseButtonHeld)
        {
            Vec2 delta {
                mousePos.x - editLastMousePos.x,
                mousePos.y - editLastMousePos.y
            };

            if (!editDragging)
            {
                double totalDist = std::hypot(
                    mousePos.x - editDragStartPos.x,
                    mousePos.y - editDragStartPos.y
                );
                if (totalDist >= EDIT_DRAG_THRESHOLD)
                {
                    editDragging         = true;
                    editDidDrag          = true;
                    editDragStartPending = true;
                }
            }

            if (editDragging)
            {
                editDragDelta.x      += delta.x;
                editDragDelta.y      += delta.y;
                editDragTotalDelta.x += delta.x;
                editDragTotalDelta.y += delta.y;
            }

            editLastMousePos = mousePos;
            return; // still in edit drag — don't feed pan
        }

        // editMouseButtonHeld is false: button not pressed, or the press
        // missed all handles and was cancelled via cancelEditPress().
        editLastMousePos = mousePos;
    }

    if (mode == InputMode::DrawPolygon || mode == InputMode::DrawRect)
        return;

    if (!dragging) return;

    // Left button no longer pans — we only track movement so a drag
    // can be distinguished from a click on release.
    Vec2 delta {
        mousePos.x - lastMousePos.x,
        mousePos.y - lastMousePos.y
    };

    if (std::abs(delta.x) > 4.0 || std::abs(delta.y) > 4.0)
        didDrag = true;

    lastMousePos = mousePos;
}

void Input::onScroll(double yOffset)
{
    zoomDelta += yOffset;
}

bool Input::hasPanDelta() const { return panDelta.x != 0.0 || panDelta.y != 0.0; }

Vec2 Input::consumePanDelta()
{
    Vec2 result = panDelta;
    panDelta    = { 0.0, 0.0 };
    return result;
}

bool Input::hasZoomDelta() const { return zoomDelta != 0.0; }

double Input::consumeZoomDelta()
{
    double result = zoomDelta;
    zoomDelta     = 0.0;
    return result;
}

bool Input::isDrawingRect() const { return drawing; }
bool Input::hasCompletedRect() const { return rectCompleted; }
void Input::consumeCompletedRect() { rectCompleted = false; }

void Input::cancelRect()
{
    drawing       = false;
    rectCompleted = false;
    mode          = InputMode::Navigate;
}

bool Input::hasCompletedPolygon() const { return polyCompleted; }

std::vector<Vec2> Input::consumeCompletedPolygon()
{
    polyCompleted = false;
    return std::move(completedPoly);
}

void Input::cancelPolygon()
{
    polyDrawing      = false;
    pendingPolyPoint = false;
    polyMapPoints.clear();
    mode = InputMode::Navigate;
}

void Input::closePolygon()
{
    completedPoly    = polyMapPoints;
    polyMapPoints.clear();
    pendingPolyPoint = false;
    polyDrawing      = false;
    polyCompleted    = true;
    mode             = InputMode::Navigate;
}

void Input::cancelEdit()
{
    editMouseButtonHeld  = false;
    editDragging         = false;
    editDragStartPending = false;
    editDragEndPending   = false;
    editDidDrag          = false;
    editClickPending     = false;
    editDragDelta        = { 0.0, 0.0 };
    editDragTotalDelta   = { 0.0, 0.0 };
    mode                 = InputMode::Navigate;
}

bool Input::hasClick() const { return clickPending; }

Vec2 Input::consumeClick()
{
    clickPending = false;
    return clickPos;
}

void Input::onMouseButtonRight(bool pressed, const Vec2& mousePos)
{
    if (pressed)
    {
        rightClickPending = true;
        rightClickPos     = mousePos;
    }
}
