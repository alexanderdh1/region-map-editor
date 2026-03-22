#pragma once

#include "data/RegionTree.h"
#include "rendering/Camera.h"
#include "input/Input.h"

class RegionRenderer
{
public:
    void render(const RegionTree& tree, const Camera& camera) const;
    void renderPreview(const Input& input, const Camera& camera) const;

private:
    void renderRegion(const Region& region, const Camera& camera) const;
    void renderRectPreview(const Input& input, const Camera& camera) const;
    void renderPolygonPreview(const Input& input, const Camera& camera) const;

    void drawFilledPolygon(
        const Camera& camera,
        const std::vector<Vec2>& worldPoints,
        float r, float g, float b, float a
    ) const;

    void drawOutline(
        const Camera& camera,
        const std::vector<Vec2>& worldPoints,
        float r, float g, float b, float a,
        bool closed = true
    ) const;
};
