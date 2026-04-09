#pragma once

#include "data/RegionTree.h"
#include "rendering/Camera.h"
#include "input/Input.h"
#include "core/Core.h"

class RegionRenderer
{
public:
    void render(const RegionTree& tree, const Camera& camera) const;
    void renderPreview(const Input& input, const Camera& camera) const;
    void renderEditHandles(const Core& core) const;

private:
    void renderRegion(const Region& region, const Camera& camera) const;
    void renderRectPreview(const Input& input, const Camera& camera) const;
    void renderPolygonPreview(const Input& input, const Camera& camera) const;

    void drawFilledPolygon(
        const Camera& camera,
        const std::vector<Vec2>& mapPoints,
        float r, float g, float b, float a
    ) const;

    void drawOutline(
        const Camera& camera,
        const std::vector<Vec2>& mapPoints,
        float r, float g, float b, float a,
        bool closed = true
    ) const;

    // Draw a single handle circle at screen position
    void drawHandle(const Vec2& screenPos,
                    float r, float g, float b,
                    float radius = 6.0f) const;
};
