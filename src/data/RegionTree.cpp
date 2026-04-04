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
    if (!parent) return nullptr;
    Region* raw = child.get();
    parent->addChild(std::move(child));
    return raw;
}

void RegionTree::removeRegion(RegionId id)
{
    removeFromList(roots_, id);
}

bool RegionTree::moveRegion(RegionId regionId, RegionId newParentId)
{
    Region* region = findById(regionId);
    if (!region) return false;

    RegionId currentParentId = region->parent ? region->parent->id : 0;
    if (currentParentId == newParentId) return false;

    if (newParentId != 0 && isAncestorOf(regionId, newParentId))
        return false;

    std::unique_ptr<Region> node = detachRegion(regionId);
    if (!node) return false;

    if (newParentId == 0)
    {
        node->parent = nullptr;
        roots_.push_back(std::move(node));
    }
    else
    {
        Region* newParent = findById(newParentId);
        if (!newParent)
        {
            // Safety: re-attach to root rather than losing the node
            node->parent = nullptr;
            roots_.push_back(std::move(node));
            return false;
        }
        newParent->addChild(std::move(node));
    }
    return true;
}

bool RegionTree::isAncestorOf(RegionId ancestorId, RegionId regionId) const
{
    if (ancestorId == regionId) return true;
    const Region* r = findById(regionId);
    if (!r) return false;
    const Region* p = r->parent;
    while (p)
    {
        if (p->id == ancestorId) return true;
        p = p->parent;
    }
    return false;
}

Region* RegionTree::findById(RegionId id) const { return findByIdIn(roots_, id); }

void RegionTree::forEach(const std::function<void(Region&)>& fn)       { forEachIn(roots_, fn); }
void RegionTree::forEach(const std::function<void(const Region&)>& fn) const { forEachIn(roots_, fn); }
const std::vector<std::unique_ptr<Region>>& RegionTree::roots() const  { return roots_; }
RegionId RegionTree::nextId() { return nextId_++; }

Region* RegionTree::findByIdIn(
    const std::vector<std::unique_ptr<Region>>& list, RegionId id) const
{
    for (const auto& r : list)
    {
        if (r->id == id) return r.get();
        Region* found = findByIdIn(r->children, id);
        if (found) return found;
    }
    return nullptr;
}

std::unique_ptr<Region> RegionTree::detachRegion(RegionId id)
{
    return detachFromList(roots_, id);
}

std::unique_ptr<Region> RegionTree::detachFromList(
    std::vector<std::unique_ptr<Region>>& list, RegionId id)
{
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        if ((*it)->id == id)
        {
            std::unique_ptr<Region> node = std::move(*it);
            list.erase(it);
            return node;
        }
        auto found = detachFromList((*it)->children, id);
        if (found) return found;
    }
    return nullptr;
}

bool RegionTree::removeFromList(
    std::vector<std::unique_ptr<Region>>& list, RegionId id)
{
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        if ((*it)->id == id) { list.erase(it); return true; }
        if (removeFromList((*it)->children, id)) return true;
    }
    return false;
}

void RegionTree::forEachIn(
    std::vector<std::unique_ptr<Region>>& list,
    const std::function<void(Region&)>& fn)
{
    for (auto& r : list) { fn(*r); forEachIn(r->children, fn); }
}

void RegionTree::forEachIn(
    const std::vector<std::unique_ptr<Region>>& list,
    const std::function<void(const Region&)>& fn) const
{
    for (const auto& r : list) { fn(*r); forEachIn(r->children, fn); }
}
