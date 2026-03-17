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

void Camera::clampToBounds(double worldWidth, double worldHeight)
{
    // Enforce dynamic minimum zoom so the world always fills the viewport
    double minZoomX = viewportSize.x / worldWidth;
    double minZoomY = viewportSize.y / worldHeight;
    double dynamicMinZoom = std::max(minZoomX, minZoomY);

    if (zoom < dynamicMinZoom)
        zoom = dynamicMinZoom;

    double halfW = worldWidth  / 2.0;
    double halfH = worldHeight / 2.0;

    double visibleHalfW = viewportSize.x / (2.0 * zoom);
    double visibleHalfH = viewportSize.y / (2.0 * zoom);

    // X axis — center if fully zoomed out
    if (visibleHalfW >= halfW)
    {
        position.x = 0.0;
    }
    else
    {
        position.x = std::clamp(
            position.x,
            -halfW + visibleHalfW,
             halfW - visibleHalfW
        );
    }

    // Y axis — center if fully zoomed out
    if (visibleHalfH >= halfH)
    {
        position.y = 0.0;
    }
    else
    {
        position.y = std::clamp(
            position.y,
            -halfH + visibleHalfH,
             halfH - visibleHalfH
        );
    }
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