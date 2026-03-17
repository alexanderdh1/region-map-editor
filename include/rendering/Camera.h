#pragma once
#include "math/Vec2.h"

class Camera {
public:
    Camera();

    Vec2 position;
    double zoom;
    Vec2 viewportSize;

    // Configuration
    double minZoom;
    double maxZoom;

    // Camera control
    void panBy(const Vec2& delta);
    void zoomBy(double factor);

    // Clamp position and zoom to world bounds
    void clampToBounds(double worldWidth, double worldHeight);

    // Coordinate conversion
    Vec2 worldToScreen(const Vec2& worldPos) const;
    Vec2 screenToWorld(const Vec2& screenPos) const;
};
