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
    if (input.isDrawingRect())
        renderRectPreview(input, camera);

    if (input.isDrawingPolygon())
        renderPolygonPreview(input, camera);
}

// ---- Private ----

void RegionRenderer::renderRegion(const Region& region, const Camera& camera) const
{
    if (!region.geometry.isValid())
        return;

    std::vector<Vec2> pts = region.geometry.getPoints();

    drawFilledPolygon(camera, pts,
        region.colorR, region.colorG, region.colorB, region.colorA);

    drawOutline(camera, pts,
        region.colorR, region.colorG, region.colorB,
        std::min(1.0f, region.colorA + 0.4f));
}

void RegionRenderer::renderRectPreview(
    const Input& input, const Camera& camera) const
{
    // Start corner is already in world space
    Vec2 worldA = input.getDrawStartWorld();
    Vec2 worldB = camera.screenToWorld(input.getDrawCurrent());

    double minX = std::min(worldA.x, worldB.x);
    double maxX = std::max(worldA.x, worldB.x);
    double minY = std::min(worldA.y, worldB.y);
    double maxY = std::max(worldA.y, worldB.y);

    std::vector<Vec2> corners = {
        { minX, minY }, { maxX, minY },
        { maxX, maxY }, { minX, maxY }
    };

    drawFilledPolygon(camera, corners, 1.0f, 1.0f, 1.0f, 0.15f);
    drawOutline(camera, corners, 1.0f, 1.0f, 1.0f, 0.9f);
}

void RegionRenderer::renderPolygonPreview(
    const Input& input, const Camera& camera) const
{
    const std::vector<Vec2>& worldPts = input.getPolygonWorldPoints();
    if (worldPts.empty()) return;

    glDisable(GL_TEXTURE_2D);

    // Lines between placed points
    if (worldPts.size() >= 2)
    {
        glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
        glLineWidth(1.2f);
        glBegin(GL_LINE_STRIP);
        for (const Vec2& p : worldPts)
        {
            Vec2 s = camera.worldToScreen(p);
            glVertex2d(s.x, s.y);
        }
        glEnd();
        glLineWidth(1.0f);
    }

    // Thin line from last point to cursor
    Vec2 cursor       = camera.screenToWorld(input.getPolygonCursor());
    Vec2 lastScreen   = camera.worldToScreen(worldPts.back());
    Vec2 cursorScreen = camera.worldToScreen(cursor);

    glColor4f(1.0f, 1.0f, 1.0f, 0.35f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2d(lastScreen.x,   lastScreen.y);
    glVertex2d(cursorScreen.x, cursorScreen.y);
    glEnd();

    // White dots at each confirmed point
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    for (const Vec2& p : worldPts)
    {
        Vec2 s = camera.worldToScreen(p);
        glVertex2d(s.x, s.y);
    }
    glEnd();
    glPointSize(1.0f);

    // Yellow dot on first point when enough points to close
    if (worldPts.size() >= 3)
    {
        Vec2 first = camera.worldToScreen(worldPts[0]);
        glColor4f(1.0f, 0.8f, 0.2f, 1.0f);
        glPointSize(9.0f);
        glBegin(GL_POINTS);
        glVertex2d(first.x, first.y);
        glEnd();
        glPointSize(1.0f);
    }
}

void RegionRenderer::drawFilledPolygon(
    const Camera& camera,
    const std::vector<Vec2>& worldPoints,
    float r, float g, float b, float a) const
{
    if (worldPoints.size() < 3) return;

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_STENCIL_TEST);

    // --- Pass 1: build stencil mask using even-odd (GL_INVERT) ---
    // Self-overlapping areas cancel out, so only odd-coverage pixels remain.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);

    Vec2 origin = camera.worldToScreen(worldPoints[0]);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(origin.x, origin.y);
    for (size_t i = 1; i < worldPoints.size(); i++)
    {
        Vec2 s = camera.worldToScreen(worldPoints[i]);
        glVertex2d(s.x, s.y);
    }
    glVertex2d(origin.x, origin.y);
    glEnd();

    // --- Pass 2: draw colour once per pixel, reset stencil after ---
    // GL_ZERO resets stencil as we draw, preventing any pixel from
    // being coloured more than once even across multiple regions.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(origin.x, origin.y);
    for (size_t i = 1; i < worldPoints.size(); i++)
    {
        Vec2 s = camera.worldToScreen(worldPoints[i]);
        glVertex2d(s.x, s.y);
    }
    glVertex2d(origin.x, origin.y);
    glEnd();

    glDisable(GL_STENCIL_TEST);
}

void RegionRenderer::drawOutline(
    const Camera& camera,
    const std::vector<Vec2>& worldPoints,
    float r, float g, float b, float a,
    bool closed) const
{
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.5f);
    glColor4f(r, g, b, a);
    glBegin(closed ? GL_LINE_LOOP : GL_LINE_STRIP);
    for (const Vec2& p : worldPoints)
    {
        Vec2 s = camera.worldToScreen(p);
        glVertex2d(s.x, s.y);
    }
    glEnd();
    glLineWidth(1.0f);
}

