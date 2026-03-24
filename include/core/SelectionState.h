#pragma once

#include <vector>
#include "data/Region.h"
#include "math/Vec2.h"

struct SelectionState
{
    // --- Main selection (left-click) ---
    Region* selectedRegion = nullptr;
    bool    popupOpen      = false;

    // Navigation stack: when viewing a sub-region popup,
    // this holds the chain of parents so we can go back.
    // viewStack[0] = top-level ancestor, back() = current popup region's parent.
    std::vector<Region*> viewStack;

    void select(Region* region)
    {
        selectedRegion = region;
        popupOpen      = (region != nullptr);
        viewStack.clear();
    }

    // Navigate into a sub-region from the popup
    void pushView(Region* child)
    {
        if (selectedRegion)
            viewStack.push_back(selectedRegion);
        selectedRegion = child;
        popupOpen      = true;
    }

    // Navigate back to parent
    void popView()
    {
        if (!viewStack.empty())
        {
            selectedRegion = viewStack.back();
            viewStack.pop_back();
            popupOpen = true;
        }
        else
        {
            clear();
        }
    }

    bool canGoBack() const { return !viewStack.empty(); }

    void clear()
    {
        selectedRegion = nullptr;
        popupOpen      = false;
        viewStack.clear();
        contextRegion    = nullptr;
        contextMenuOpen  = false;
    }

    bool hasSelection() const { return selectedRegion != nullptr; }

    // --- Context menu (right-click) ---
    Region* contextRegion     = nullptr;
    bool    contextMenuOpen   = false;
    Vec2    contextMenuScreen { 0.0, 0.0 };

    void closeContextMenu()
    {
        contextRegion   = nullptr;
        contextMenuOpen = false;
    }
};
