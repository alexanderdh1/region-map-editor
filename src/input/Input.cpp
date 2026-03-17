#include "input/Input.h"
#include <cmath>

void Input::onMouseButton(bool pressed, const Vec2& mousePos, bool shiftHeld)
{
    if (pressed)
    {
        mode = shiftHeld ? InputMode::DrawRect : InputMode::Navigate;

        if (mode == InputMode::DrawRect)
        {
            drawing       = true;
            rectCompleted = false;
            drawStart     = mousePos;
            drawCurrent   = mousePos;
        }
        else
        {
            dragging     = true;
            didDrag      = false;
            lastMousePos = mousePos;
            clickPos     = mousePos;
        }
    }
    else // released
    {
        if (mode == InputMode::DrawRect && drawing)
        {
            drawing       = false;
            rectCompleted = true;
        }
        else
        {
            // Only register a click if the mouse didn't move significantly
            if (!didDrag)
            {
                clickPending = true;
            }
            dragging = false;
            didDrag  = false;
        }

        mode = InputMode::Navigate;
    }
}

void Input::onMouseMove(const Vec2& mousePos)
{
    if (drawing)
    {
        drawCurrent = mousePos;
        return;
    }

    if (!dragging) return;

    Vec2 delta{
        mousePos.x - lastMousePos.x,
        mousePos.y - lastMousePos.y
    };

    // Consider it a drag if moved more than 4 pixels
    if (std::abs(delta.x) > 4.0 || std::abs(delta.y) > 4.0)
        didDrag = true;

    panDelta.x += delta.x;
    panDelta.y += delta.y;

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

bool Input::hasZoomDelta() const
{
    return zoomDelta != 0.0;
}

double Input::consumeZoomDelta()
{
    double result = zoomDelta;
    zoomDelta     = 0.0;
    return result;
}

// --- Drawing ---

bool Input::isDrawingRect() const
{
    return drawing;
}

bool Input::hasCompletedRect() const
{
    return rectCompleted;
}

void Input::consumeCompletedRect()
{
    rectCompleted = false;
}

// --- Click ---

bool Input::hasClick() const
{
    return clickPending;
}

Vec2 Input::consumeClick()
{
    clickPending = false;
    return clickPos;
}
