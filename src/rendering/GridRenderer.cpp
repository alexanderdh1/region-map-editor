#include "rendering/GridRenderer.h"

#include <GLFW/glfw3.h>

void GridRenderer::render(const Camera& camera) const {

    glLoadIdentity();
    glColor3f(0.25f, 0.25f, 0.28f);

    // Vertical lines
    for (int i = -gridHalfSize; i <= gridHalfSize; ++i) {
        double x = i * gridSpacing;

        Vec2 a = camera.worldToScreen({ x, -gridHalfSize * gridSpacing });
        Vec2 b = camera.worldToScreen({ x,  gridHalfSize * gridSpacing });

        glBegin(GL_LINES);
        glVertex2d(a.x, a.y);
        glVertex2d(b.x, b.y);
        glEnd();
    }

    // Horizontal lines
    for (int i = -gridHalfSize; i <= gridHalfSize; ++i) {
        double y = i * gridSpacing;

        Vec2 a = camera.worldToScreen({ -gridHalfSize * gridSpacing, y });
        Vec2 b = camera.worldToScreen({  gridHalfSize * gridSpacing, y });

        glBegin(GL_LINES);
        glVertex2d(a.x, a.y);
        glVertex2d(b.x, b.y);
        glEnd();
    }
}