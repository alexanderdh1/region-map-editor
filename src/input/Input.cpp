#include "input/Input.h"
#include <cmath>
#include <GLFW/glfw3.h>

static constexpr double DOUBLE_CLICK_SECONDS = 0.35;
static constexpr double DOUBLE_CLICK_RADIUS  = 10.0;

void Input::onMouseButton(bool pressed, const Vec2& mousePos, bool shiftHeld)
{
    if (pressed)
    {
        // ---- POLYGON MODE ----
        if (shiftHeld && activeTool == DrawTool::Polygon)
        {
            mode = InputMode::DrawPolygon;

            double now = glfwGetTime();
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
        if (mode == InputMode::DrawRect && drawing)
        {
            drawing       = false;
            rectCompleted = true;
            mode          = InputMode::Navigate;
            return;
        }

        if (mode == InputMode::DrawPolygon)
            return; // polygon points are placed on press, not release

        // Navigate release — register click if no drag occurred
        if (!didDrag)
            clickPending = true;

        dragging = false;
        didDrag  = false;
        mode     = InputMode::Navigate;
    }
}

void Input::onMouseMove(const Vec2& mousePos)
{
    // Update cursor for both rect and polygon preview
    drawCurrent = mousePos;

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

// --- Rectangle ---

bool Input::isDrawingRect() const { return drawing; }

bool Input::hasCompletedRect() const { return rectCompleted; }

void Input::consumeCompletedRect() { rectCompleted = false; }

void Input::cancelRect()
{
    drawing       = false;
    rectCompleted = false;
    mode          = InputMode::Navigate;
}

// --- Polygon ---

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
    completedPoly  = polyWorldPoints;
    polyWorldPoints.clear();
    pendingPolyPoint = false;
    polyDrawing    = false;
    polyCompleted  = true;
    mode           = InputMode::Navigate;
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

