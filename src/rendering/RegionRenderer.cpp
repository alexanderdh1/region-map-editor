#include "rendering/RegionRenderer.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

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

// ---------------------------------------------------------------
// Edit handles
// ---------------------------------------------------------------

void RegionRenderer::renderEditHandles(const Core& core) const
{
    const EditState& edit   = core.getEditState();
    const Camera&    camera = core.getCamera();

    if (!edit.isActive()) return;

    const Region& region = *edit.target;

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Highlight outline
    auto pts = region.geometry.getPoints();
    glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    for (const Vec2& p : pts)
    {
        Vec2 s = camera.worldToScreen(p);
        glVertex2d(s.x, s.y);
    }
    glEnd();
    glLineWidth(1.0f);

    if (region.geometry.type == GeometryType::Rectangle)
    {
        Vec2 corners[4] = {
            camera.worldToScreen({ region.geometry.rectMin.x, region.geometry.rectMax.y }),
            camera.worldToScreen({ region.geometry.rectMax.x, region.geometry.rectMax.y }),
            camera.worldToScreen({ region.geometry.rectMax.x, region.geometry.rectMin.y }),
            camera.worldToScreen({ region.geometry.rectMin.x, region.geometry.rectMin.y }),
        };
        for (int i = 0; i < 4; i++)
        {
            bool active  = (edit.handleType == EditHandleType::RectCorner &&
                            edit.handleIndex == i);
            bool hovered = (!active && edit.hoveredHandleIndex == i);
            drawHandle(corners[i],
                active  ? 1.0f  : hovered ? 1.0f  : 1.0f,
                active  ? 0.85f : hovered ? 0.85f : 1.0f,
                active  ? 0.2f  : hovered ? 0.0f  : 1.0f,
                active  ? 8.0f  : hovered ? 7.5f  : 6.0f);
        }
    }
    else if (region.geometry.type == GeometryType::Polygon)
    {
        const auto& polyPts = region.geometry.points;
        for (int i = 0; i < static_cast<int>(polyPts.size()); i++)
        {
            Vec2 s = camera.worldToScreen(polyPts[i]);
            bool active  = (edit.handleType == EditHandleType::PolyPoint &&
                            edit.handleIndex == i);
            bool hovered = (!active && edit.hoveredHandleIndex == i);
            drawHandle(s,
                active  ? 1.0f  : hovered ? 1.0f  : 1.0f,
                active  ? 0.85f : hovered ? 0.85f : 1.0f,
                active  ? 0.2f  : hovered ? 0.0f  : 1.0f,
                active  ? 8.0f  : hovered ? 7.5f  : 6.0f);
        }
    }
}

void RegionRenderer::drawHandle(const Vec2& screenPos,
                                float r, float g, float b,
                                float radius) const
{
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    static constexpr int    SEG    = 16;
    static constexpr double TWO_PI = 6.28318530718;

    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(screenPos.x, screenPos.y);
    for (int i = 0; i <= SEG; i++)
    {
        double a = TWO_PI * i / SEG;
        glVertex2d(screenPos.x + std::cos(a) * (radius + 1.5),
                   screenPos.y + std::sin(a) * (radius + 1.5));
    }
    glEnd();

    glColor4f(r, g, b, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(screenPos.x, screenPos.y);
    for (int i = 0; i <= SEG; i++)
    {
        double a = TWO_PI * i / SEG;
        glVertex2d(screenPos.x + std::cos(a) * radius,
                   screenPos.y + std::sin(a) * radius);
    }
    glEnd();
}

// ---------------------------------------------------------------
// Region rendering
// ---------------------------------------------------------------

void RegionRenderer::renderRegion(const Region& region, const Camera& camera) const
{
    if (!region.geometry.isValid()) return;
    if (region.hidden) return; // hidden regions are not rendered

    std::vector<Vec2> pts = region.geometry.getPoints();

    float depth = 0.0f;
    const Region* p = region.parent;
    while (p) { depth += 1.0f; p = p->parent; }

    float fillAlpha    = region.colorA * (1.0f - depth * 0.08f);
    float outlineWidth = 1.5f + depth * 1.0f;

    drawFilledPolygon(camera, pts,
        region.colorR, region.colorG, region.colorB,
        std::max(0.1f, fillAlpha));

    // Explicit state reset after stencil operations
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(outlineWidth);
    drawOutline(camera, pts,
        region.colorR, region.colorG, region.colorB,
        std::min(1.0f, region.colorA + 0.4f));
    glLineWidth(1.0f);

    for (const auto& child : region.children)
        renderRegion(*child, camera);
}

void RegionRenderer::renderRectPreview(
    const Input& input, const Camera& camera) const
{
    Vec2 mapA = input.getDrawStartMap();
    Vec2 cur    = input.getDrawCurrent();
    cur.x = std::max(0.0, std::min(cur.x, camera.viewportSize.x));
    cur.y = std::max(0.0, std::min(cur.y, camera.viewportSize.y));
    Vec2 mapB = camera.screenToWorld(cur);

    std::vector<Vec2> corners = {
        { std::min(mapA.x, mapB.x), std::min(mapA.y, mapB.y) },
        { std::max(mapA.x, mapB.x), std::min(mapA.y, mapB.y) },
        { std::max(mapA.x, mapB.x), std::max(mapA.y, mapB.y) },
        { std::min(mapA.x, mapB.x), std::max(mapA.y, mapB.y) },
    };

    drawFilledPolygon(camera, corners, 1.0f, 1.0f, 1.0f, 0.15f);

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawOutline(camera, corners, 1.0f, 1.0f, 1.0f, 0.9f);
}

void RegionRenderer::renderPolygonPreview(
    const Input& input, const Camera& camera) const
{
    const std::vector<Vec2>& mapPts = input.getPolygonMapPoints();
    if (mapPts.empty()) return;

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (mapPts.size() >= 2)
    {
        glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
        glLineWidth(1.2f);
        glBegin(GL_LINE_STRIP);
        for (const Vec2& p : mapPts)
        {
            Vec2 s = camera.worldToScreen(p);
            glVertex2d(s.x, s.y);
        }
        glEnd();
        glLineWidth(1.0f);
    }

    Vec2 lastScreen   = camera.worldToScreen(mapPts.back());
    Vec2 cursorScreen = input.getPolygonCursor();

    glColor4f(1.0f, 1.0f, 1.0f, 0.35f);
    glBegin(GL_LINES);
    glVertex2d(lastScreen.x,   lastScreen.y);
    glVertex2d(cursorScreen.x, cursorScreen.y);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    for (const Vec2& p : mapPts)
    {
        Vec2 s = camera.worldToScreen(p);
        glVertex2d(s.x, s.y);
    }
    glEnd();
    glPointSize(1.0f);

    if (mapPts.size() >= 3)
    {
        Vec2 first = camera.worldToScreen(mapPts[0]);
        glColor4f(1.0f, 0.8f, 0.2f, 1.0f);
        glPointSize(9.0f);
        glBegin(GL_POINTS);
        glVertex2d(first.x, first.y);
        glEnd();
        glPointSize(1.0f);
    }
}

// ---------------------------------------------------------------
// Filled polygon — original even-odd stencil (GL_INVERT)
// ---------------------------------------------------------------

void RegionRenderer::drawFilledPolygon(
    const Camera& camera,
    const std::vector<Vec2>& mapPoints,
    float r, float g, float b, float a) const
{
    if (mapPoints.size() < 3) return;

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_STENCIL_TEST);

    // Pass 1: stencil mask via even-odd (GL_INVERT)
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);

    Vec2 origin = camera.worldToScreen(mapPoints[0]);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(origin.x, origin.y);
    for (size_t i = 1; i < mapPoints.size(); i++)
    {
        Vec2 s = camera.worldToScreen(mapPoints[i]);
        glVertex2d(s.x, s.y);
    }
    glVertex2d(origin.x, origin.y);
    glEnd();

    // Pass 2: colour where stencil != 0, reset stencil as we go
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(origin.x, origin.y);
    for (size_t i = 1; i < mapPoints.size(); i++)
    {
        Vec2 s = camera.worldToScreen(mapPoints[i]);
        glVertex2d(s.x, s.y);
    }
    glVertex2d(origin.x, origin.y);
    glEnd();

    glDisable(GL_STENCIL_TEST);
}

void RegionRenderer::drawOutline(
    const Camera& camera,
    const std::vector<Vec2>& mapPoints,
    float r, float g, float b, float a,
    bool closed) const
{
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);
    glBegin(closed ? GL_LINE_LOOP : GL_LINE_STRIP);
    for (const Vec2& p : mapPoints)
    {
        Vec2 s = camera.worldToScreen(p);
        glVertex2d(s.x, s.y);
    }
    glEnd();
}
