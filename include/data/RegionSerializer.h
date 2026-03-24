#pragma once

#include <string>
#include "data/RegionTree.h"

// Serializes and deserializes the entire RegionTree to/from a JSON file.
//
// File format: regions.json in the same folder as the executable.
// The format is human-readable and versioned for future compatibility.

class RegionSerializer
{
public:
    // Save all regions to file. Returns true on success.
    static bool save(const RegionTree& tree, const std::string& path);

    // Load regions from file into tree (clears existing regions first).
    // Returns true on success.
    static bool load(RegionTree& tree, const std::string& path);
};
