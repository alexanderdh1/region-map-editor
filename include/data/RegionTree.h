#pragma once

#include <vector>
#include <memory>
#include <functional>

#include "data/Region.h"

// RegionTree owns all top-level regions and is the single source of
// truth for region data. Sub-regions are owned by their parent Region.
class RegionTree
{
public:
    // Add a new top-level region. Returns a raw pointer for immediate use.
    Region* addRegion(std::unique_ptr<Region> region);

    // Add a sub-region under an existing parent by id.
    // Returns nullptr if parent is not found.
    Region* addChildRegion(RegionId parentId, std::unique_ptr<Region> child);

    // Remove a region (and all its children) by id.
    void removeRegion(RegionId id);

    // Find a region by id anywhere in the tree.
    Region* findById(RegionId id) const;

    // Traverse every region in the tree (depth-first).
    void forEach(const std::function<void(Region&)>& fn);
    void forEach(const std::function<void(const Region&)>& fn) const;

    // Direct access to top-level regions (e.g. for rendering)
    const std::vector<std::unique_ptr<Region>>& roots() const;

    // Generate a unique id for a new region
    RegionId nextId();

private:
    std::vector<std::unique_ptr<Region>> roots_;
    RegionId nextId_ = 1;

    // Internal recursive helpers
    Region* findByIdIn(
        const std::vector<std::unique_ptr<Region>>& list,
        RegionId id
    ) const;

    bool removeFromList(
        std::vector<std::unique_ptr<Region>>& list,
        RegionId id
    );

    void forEachIn(
        std::vector<std::unique_ptr<Region>>& list,
        const std::function<void(Region&)>& fn
    );

    void forEachIn(
        const std::vector<std::unique_ptr<Region>>& list,
        const std::function<void(const Region&)>& fn
    ) const;
};
