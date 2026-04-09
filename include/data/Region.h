#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "data/RegionGeometry.h"

using RegionId = uint32_t;

struct Region
{
    RegionId    id   = 0;
    std::string name = "Unnamed Region";
    std::string note;

    // Display colour as RGB floats [0.0 - 1.0]
    float colorR = 0.4f;
    float colorG = 0.6f;
    float colorB = 1.0f;
    float colorA = 0.35f;

    // hidden    = not rendered on map (circle goes grey in tree)
    // collapsed = children hidden in tree view
    bool hidden    = false;
    bool collapsed = false;

    RegionGeometry geometry;

    // Sub-regions — this region owns its children
    std::vector<std::unique_ptr<Region>> children;

    // Non-owning pointer to parent (nullptr = top-level region)
    Region* parent = nullptr;

    bool isTopLevel() const { return parent == nullptr; }

    void addChild(std::unique_ptr<Region> child)
    {
        child->parent = this;
        children.push_back(std::move(child));
    }
};
