#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "data/RegionGeometry.h"
#include "data/RegionStatus.h"
#include "data/Marker.h"

using RegionId = uint32_t;

struct Region
{
    RegionId        id       = 0;
    std::string     name     = "Unnamed Region";
    std::string     note;
    RegionStatus    status   = RegionStatus::None;

    // Display colour as RGB floats [0.0 - 1.0]
    float colorR = 0.4f;
    float colorG = 0.6f;
    float colorB = 1.0f;
    float colorA = 0.35f;  // fill opacity

    RegionGeometry geometry;

    std::vector<Marker> markers;

    // Sub-regions — this region owns its children
    std::vector<std::unique_ptr<Region>> children;

    // Non-owning pointer to parent (nullptr = top-level region)
    Region* parent = nullptr;

    // --- Helpers ---

    bool isTopLevel() const { return parent == nullptr; }

    void addChild(std::unique_ptr<Region> child)
    {
        child->parent = this;
        children.push_back(std::move(child));
    }

    void addMarker(Marker marker)
    {
        markers.push_back(std::move(marker));
    }
};
