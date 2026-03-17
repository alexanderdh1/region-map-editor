#include "data/RegionTree.h"

Region* RegionTree::addRegion(std::unique_ptr<Region> region)
{
    Region* raw = region.get();
    raw->parent = nullptr;
    roots_.push_back(std::move(region));
    return raw;
}

Region* RegionTree::addChildRegion(RegionId parentId, std::unique_ptr<Region> child)
{
    Region* parent = findById(parentId);
    if (!parent)
        return nullptr;

    Region* raw = child.get();
    parent->addChild(std::move(child));
    return raw;
}

void RegionTree::removeRegion(RegionId id)
{
    removeFromList(roots_, id);
}

Region* RegionTree::findById(RegionId id) const
{
    return findByIdIn(roots_, id);
}

void RegionTree::forEach(const std::function<void(Region&)>& fn)
{
    forEachIn(roots_, fn);
}

void RegionTree::forEach(const std::function<void(const Region&)>& fn) const
{
    forEachIn(roots_, fn);
}

const std::vector<std::unique_ptr<Region>>& RegionTree::roots() const
{
    return roots_;
}

RegionId RegionTree::nextId()
{
    return nextId_++;
}

// ---- Private helpers ----

Region* RegionTree::findByIdIn(
    const std::vector<std::unique_ptr<Region>>& list,
    RegionId id
) const
{
    for (const auto& r : list)
    {
        if (r->id == id)
            return r.get();

        Region* found = findByIdIn(r->children, id);
        if (found)
            return found;
    }
    return nullptr;
}

bool RegionTree::removeFromList(
    std::vector<std::unique_ptr<Region>>& list,
    RegionId id
)
{
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        if ((*it)->id == id)
        {
            list.erase(it);
            return true;
        }

        if (removeFromList((*it)->children, id))
            return true;
    }
    return false;
}

void RegionTree::forEachIn(
    std::vector<std::unique_ptr<Region>>& list,
    const std::function<void(Region&)>& fn
)
{
    for (auto& r : list)
    {
        fn(*r);
        forEachIn(r->children, fn);
    }
}

void RegionTree::forEachIn(
    const std::vector<std::unique_ptr<Region>>& list,
    const std::function<void(const Region&)>& fn
) const
{
    for (const auto& r : list)
    {
        fn(*r);
        forEachIn(r->children, fn);
    }
}
