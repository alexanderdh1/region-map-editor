#pragma once

#include <string>
#include "data/RegionTree.h"

class Core;

class RegionSerializer
{
public:
    // Save all regions to file using the coordinate mode from core
    // (map coordinates when metadata is present, normalised 0-1 for image maps).
    static bool save(const RegionTree& tree,
                     const std::string& path,
                     const Core& core);

    // Load regions from file and convert coordinates to map-space
    // using the current map dimensions from core.
    static bool load(RegionTree& tree,
                     const std::string& path,
                     const Core& core);
};
