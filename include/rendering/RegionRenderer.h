#pragma once

#include "data/RegionTree.h"
#include "rendering/Camera.h"
#include "input/Input.h"

class RegionRenderer
{
public:
    // Draw all regions in the tree
    void render(const RegionTree& tree, const Camera& camera) const;

    // Draw the live rectangle preview while the user is dragging
    void renderPreview(const Input& input, const Camera& camera) const;

private:
    void renderRegion(const Region& region, const Camera& camera) const;

    // Draw a filled + outlined quad from four screen-space corners
    void drawQuad(
        const Camera& camera,
        const std::vector<Vec2>& worldPoints,
        float r, float g, float b, float a
    ) const;
};
