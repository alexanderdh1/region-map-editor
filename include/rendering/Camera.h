#pragma once
#include "math/Vec2.h"

class Camera {
public:
    Camera();

    Vec2   position;
    double zoom;
    Vec2   viewportSize;

    double minZoom;
    double maxZoom;

    void panBy(const Vec2& delta);
    void zoomBy(double factor);

    // Clamp position and zoom to world bounds.
    // Also caches the world size so panBy can hard-clamp immediately.
    void clampToBounds(double worldWidth, double worldHeight);

    Vec2 worldToScreen(const Vec2& worldPos) const;
    Vec2 screenToWorld(const Vec2& screenPos) const;

private:
    // Cached world size — set by clampToBounds, used by panBy
    double worldWidth_  = 0.0;
    double worldHeight_ = 0.0;
};
