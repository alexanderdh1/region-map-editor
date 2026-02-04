#pragma once
#include "math/Vec2.h"

class Input {
public:
    // Call when mouse button is pressed or released
    void onMouseButton(bool pressed, const Vec2& mousePos);

    // Call when mouse moves
    void onMouseMove(const Vec2& mousePos);

    // Check if there is accumulated pan movement
    bool hasPanDelta() const;

    // Get and clear accumulated pan movement
    Vec2 consumePanDelta();

private:
    bool dragging = false;
    Vec2 lastMousePos{0.0, 0.0};
    Vec2 panDelta{0.0, 0.0};
};
