#pragma once
#include "math/Vec2.h"

enum class InputMode
{
    Navigate, // Default: pan and zoom
    DrawRect  // Shift+drag: draw a rectangle region
};

class Input {
public:
    // Called when mouse button is pressed or released.
    // shiftHeld determines whether we enter draw mode.
    void onMouseButton(bool pressed, const Vec2& mousePos, bool shiftHeld);

    void onMouseMove(const Vec2& mousePos);
    void onScroll(double yOffset);

    // --- Navigation ---
    bool hasPanDelta() const;
    Vec2 consumePanDelta();

    bool hasZoomDelta() const;
    double consumeZoomDelta();

    // --- Drawing ---
    InputMode getMode() const { return mode; }

    bool isDrawingRect() const;

    Vec2 getDrawStart()   const { return drawStart; }
    Vec2 getDrawCurrent() const { return drawCurrent; }

    bool hasCompletedRect() const;
    void consumeCompletedRect();

    // --- Click (single press without drag, no shift) ---
    // True for exactly one frame when a clean click is released.
    bool hasClick() const;
    Vec2 consumeClick();

private:
    InputMode mode = InputMode::Navigate;

    // Navigate state
    bool   dragging     = false;
    bool   didDrag      = false;   // true if mouse moved during press
    Vec2   lastMousePos { 0.0, 0.0 };
    Vec2   panDelta     { 0.0, 0.0 };
    double zoomDelta    = 0.0;

    // Draw state
    bool drawing       = false;
    bool rectCompleted = false;
    Vec2 drawStart     { 0.0, 0.0 };
    Vec2 drawCurrent   { 0.0, 0.0 };

    // Click state
    bool clickPending  = false;
    Vec2 clickPos      { 0.0, 0.0 };
};
