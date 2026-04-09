#pragma once

#include <vector>
#include "data/Region.h"
#include "math/Vec2.h"

struct SelectionState
{
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

    void pushView(Region* child)
    {
        if (selectedRegion)
            viewStack.push_back(selectedRegion);
        selectedRegion = child;
        popupOpen      = true;
    }

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
    }

    bool hasSelection() const { return selectedRegion != nullptr; }
};
