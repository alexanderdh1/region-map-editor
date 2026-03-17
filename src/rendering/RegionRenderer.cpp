#include "rendering/RegionRenderer.h"

#include <GLFW/glfw3.h>
#include <algorithm>

void RegionRenderer::render(const RegionTree& tree, const Camera& camera) const
{
    tree.forEach([&](const Region& region)
    {
        renderRegion(region, camera);
    });
}

void RegionRenderer::renderPreview(const Input& input, const Camera& camera) const
{
    if (!input.isDrawingRect())
        return;

    Vec2 screenA = input.getDrawStart();
    Vec2 screenB = input.getDrawCurrent();

    Vec2 worldA = camera.screenToWorld(screenA);
    Vec2 worldB = camera.screenToWorld(screenB);

    double minX = std::min(worldA.x, worldB.x);
    double maxX = std::max(worldA.x, worldB.x);
    double minY = std::min(worldA.y, worldB.y);
    double maxY = std::max(worldA.y, worldB.y);

    std::vector<Vec2> corners = {
        { minX, minY },
        { maxX, minY },
        { maxX, maxY },
        { minX, maxY }
    };

    // Semi-transparent white fill
    drawQuad(camera, corners, 1.0f, 1.0f, 1.0f, 0.15f);

    // Dashed-style bright outline (drawn as solid for now)
    glLineWidth(1.5f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    glBegin(GL_LINE_LOOP);
    for (const Vec2& p : corners)
    {
        Vec2 s = camera.worldToScreen(p);
        glVertex2d(s.x, s.y);
    }
    glEnd();
    glLineWidth(1.0f);
}

// ---- Private ----

void RegionRenderer::renderRegion(const Region& region, const Camera& camera) const
{
    if (!region.geometry.isValid())
        return;

    std::vector<Vec2> pts = region.geometry.getPoints();

    // Filled polygon
    drawQuad(camera, pts,
        region.colorR,
        region.colorG,
        region.colorB,
        region.colorA);

    // Outline — slightly more opaque than fill
    glLineWidth(1.5f);
    glColor4f(region.colorR, region.colorG, region.colorB,
              std::min(1.0f, region.colorA + 0.4f));
    glBegin(GL_LINE_LOOP);
    for (const Vec2& p : pts)
    {
        Vec2 s = camera.worldToScreen(p);
        glVertex2d(s.x, s.y);
    }
    glEnd();
    glLineWidth(1.0f);
}

void RegionRenderer::drawQuad(
    const Camera& camera,
    const std::vector<Vec2>& worldPoints,
    float r, float g, float b, float a
) const
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);

    glColor4f(r, g, b, a);
    glBegin(GL_POLYGON);
    for (const Vec2& p : worldPoints)
    {
        Vec2 s = camera.worldToScreen(p);
        glVertex2d(s.x, s.y);
    }
    glEnd();
}
