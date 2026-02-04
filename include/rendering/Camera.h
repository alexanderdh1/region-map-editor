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

    // Coordinate conversion
    Vec2 worldToScreen(const Vec2& worldPos) const;
    Vec2 screenToWorld(const Vec2& screenPos) const;
};
