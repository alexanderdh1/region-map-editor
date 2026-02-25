#include "rendering/Camera.h"
#include <algorithm>

Camera::Camera()
    : position{0.0, 0.0},
      zoom{1.0},
      viewportSize{800.0, 600.0},
      minZoom{0.1},
      maxZoom{10.0}
{
}

void Camera::panBy(const Vec2& delta) {
    position.x -= delta.x / zoom;
    position.y += delta.y / zoom;
}

void Camera::zoomBy(double factor) {
    zoom *= factor;
    zoom = std::clamp(zoom, minZoom, maxZoom);
}

Vec2 Camera::worldToScreen(const Vec2& worldPos) const {
    return {
        (worldPos.x - position.x) * zoom + viewportSize.x * 0.5,
        viewportSize.y * 0.5 - (worldPos.y - position.y) * zoom
    };
}

Vec2 Camera::screenToWorld(const Vec2& screenPos) const {
    return {
        (screenPos.x - viewportSize.x * 0.5) / zoom + position.x,
        (viewportSize.y * 0.5 - screenPos.y) / zoom + position.y
    };
}