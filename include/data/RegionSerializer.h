#pragma once

#include <string>
#include "data/RegionTree.h"

class Core;

class RegionSerializer
{
public:
    // Save all regions to file using the coordinate mode from core
    // (block coordinates for Minecraft, normalised 0-1 for image maps).
    static bool save(const RegionTree& tree,
                     const std::string& path,
                     const Core& core);

    // Load regions from file and convert coordinates to world-space
    // using the current world dimensions from core.
    static bool load(RegionTree& tree,
                     const std::string& path,
                     const Core& core);
};
