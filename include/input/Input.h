#pragma once
#include "math/Vec2.h"

class Input {
public:
    // Call when mouse button is pressed or released
    void onMouseButton(bool pressed, const Vec2& mousePos);

    // Call when mouse moves
    void onMouseMove(const Vec2& mousePos);

    void onScroll(double yOffset);

    // Check if there is accumulated pan movement
    bool hasPanDelta() const;

    // Get and clear accumulated pan movement
    Vec2 consumePanDelta();

    bool hasZoomDelta() const;
    double consumeZoomDelta();

private:
    bool dragging = false;
    Vec2 lastMousePos{0.0, 0.0};
    Vec2 panDelta{0.0, 0.0};

    double zoomDelta = 0.0;
};
