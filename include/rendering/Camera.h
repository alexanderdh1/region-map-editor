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

    // Clamp position and zoom to map bounds.
    // Also caches the map size so panBy can hard-clamp immediately.
    void clampToBounds(double mapWidth, double mapHeight);

    Vec2 worldToScreen(const Vec2& worldPos) const;
    Vec2 screenToWorld(const Vec2& screenPos) const;

private:
    // Cached map size — set by clampToBounds, used by panBy
    double mapWidth_ = 0.0;
    double mapHeight_ = 0.0;
};
