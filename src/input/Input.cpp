#include "input/Input.h"
#include <cmath>
#include <GLFW/glfw3.h>

static constexpr double DOUBLE_CLICK_SECONDS = 0.35;
static constexpr double DOUBLE_CLICK_RADIUS  = 10.0;
static constexpr double EDIT_DRAG_THRESHOLD  = 3.0; // pixels before drag registers

void Input::onMouseButton(bool pressed, const Vec2& mousePos, bool shiftHeld)
{
    if (pressed)
    {
        // ---- EDIT MODE ----
        if (activeTool == DrawTool::Edit)
        {
            mode              = InputMode::Edit;
            editDragging      = false;
            editDidDrag       = false;
            editDragStartPos  = mousePos;
            editLastMousePos  = mousePos;
            editDragDelta     = { 0.0, 0.0 };
            // Drag start is confirmed lazily once threshold is exceeded (in onMouseMove)
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

            if (isDoubleClick && polyDrawing && polyWorldPoints.size() >= 3)
            {
                completedPoly    = polyWorldPoints;
                polyWorldPoints.clear();
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
            if (editDragging)
            {
                editDragging      = false;
                editDragEndPending = true;
            }
            else if (!editDidDrag)
            {
                // No drag occurred — treat as a plain click (target select / edge insert)
                editClickPending = true;
                editClickPos     = mousePos;
            }
            editDidDrag = false;
            mode = InputMode::Navigate;
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

        // Navigate release
        if (!didDrag)
            clickPending = true;

        dragging = false;
        didDrag  = false;
        mode     = InputMode::Navigate;
    }
}

void Input::onMouseMove(const Vec2& mousePos)
{
    drawCurrent = mousePos;

    // Edit mode drag accumulation
    if (activeTool == DrawTool::Edit)
    {
        Vec2 delta {
            mousePos.x - editLastMousePos.x,
            mousePos.y - editLastMousePos.y
        };

        // Check if we've exceeded the drag threshold
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
            editDragDelta.x += delta.x;
            editDragDelta.y += delta.y;
        }

        editLastMousePos = mousePos;
        return;
    }

    if (mode == InputMode::DrawPolygon || mode == InputMode::DrawRect)
        return;

    if (!dragging) return;

    Vec2 delta {
        mousePos.x - lastMousePos.x,
        mousePos.y - lastMousePos.y
    };

    if (std::abs(delta.x) > 4.0 || std::abs(delta.y) > 4.0)
        didDrag = true;

    panDelta.x  += delta.x;
    panDelta.y  += delta.y;
    lastMousePos = mousePos;
}

void Input::onScroll(double yOffset)
{
    zoomDelta += yOffset;
}

// --- Navigation ---

bool Input::hasPanDelta() const
{
    return panDelta.x != 0.0 || panDelta.y != 0.0;
}

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

// --- Rectangle drawing ---

bool Input::isDrawingRect() const { return drawing; }

bool Input::hasCompletedRect() const { return rectCompleted; }

void Input::consumeCompletedRect() { rectCompleted = false; }

void Input::cancelRect()
{
    drawing       = false;
    rectCompleted = false;
    mode          = InputMode::Navigate;
}

// --- Polygon drawing ---

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
    polyWorldPoints.clear();
    mode = InputMode::Navigate;
}

void Input::closePolygon()
{
    completedPoly = polyWorldPoints;
    polyWorldPoints.clear();
    pendingPolyPoint = false;
    polyDrawing      = false;
    polyCompleted    = true;
    mode             = InputMode::Navigate;
}

// --- Edit mode ---

void Input::cancelEdit()
{
    editDragging         = false;
    editDragStartPending = false;
    editDragEndPending   = false;
    editDidDrag          = false;
    editClickPending     = false;
    editDragDelta        = { 0.0, 0.0 };
    mode                 = InputMode::Navigate;
}

// --- Click ---

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
