#pragma once

#include <deque>
#include <string>

class RegionTree;
class Core;

// Snapshot-based undo/redo for the region tree.
// Before any mutation, call pushSnapshot() to capture the current state.
// undo() and redo() swap between stacks and restore the tree.
class HistoryManager
{
public:
    static constexpr int MAX_HISTORY = 100;

    // Capture the current tree state. Clears the redo stack.
    void pushSnapshot(const RegionTree& tree, const Core& core);

    bool undo(RegionTree& tree, const Core& core);
    bool redo(RegionTree& tree, const Core& core);

    bool canUndo() const { return !undoStack_.empty(); }
    bool canRedo() const { return !redoStack_.empty(); }

private:
    std::deque<std::string> undoStack_;
    std::deque<std::string> redoStack_;
};
