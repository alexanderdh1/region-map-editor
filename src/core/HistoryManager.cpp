#include "core/HistoryManager.h"
#include "data/RegionSerializer.h"
#include "data/RegionTree.h"
#include "core/Core.h"

void HistoryManager::pushSnapshot(const RegionTree& tree, const Core& core)
{
    undoStack_.push_back(RegionSerializer::toJson(tree, core));
    if (static_cast<int>(undoStack_.size()) > MAX_HISTORY)
        undoStack_.pop_front();
    redoStack_.clear();
}

bool HistoryManager::undo(RegionTree& tree, const Core& core)
{
    if (undoStack_.empty()) return false;
    redoStack_.push_back(RegionSerializer::toJson(tree, core));
    RegionSerializer::fromJson(undoStack_.back(), tree, core);
    undoStack_.pop_back();
    return true;
}

bool HistoryManager::redo(RegionTree& tree, const Core& core)
{
    if (redoStack_.empty()) return false;
    undoStack_.push_back(RegionSerializer::toJson(tree, core));
    RegionSerializer::fromJson(redoStack_.back(), tree, core);
    redoStack_.pop_back();
    return true;
}
