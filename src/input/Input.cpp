#include "input/Input.h"

void Input::onMouseButton(bool pressed, const Vec2& mousePos) {
    dragging = pressed;
    lastMousePos = mousePos;
}

void Input::onMouseMove(const Vec2& mousePos) {
    if (!dragging) return;

    Vec2 delta{
        mousePos.x - lastMousePos.x,
        mousePos.y - lastMousePos.y
    };

    panDelta.x += delta.x;
    panDelta.y += delta.y;

    lastMousePos = mousePos;
}

bool Input::hasPanDelta() const {
    return panDelta.x != 0.0 || panDelta.y != 0.0;
}

Vec2 Input::consumePanDelta() {
    Vec2 result = panDelta;
    panDelta = {0.0, 0.0};
    return result;
}
