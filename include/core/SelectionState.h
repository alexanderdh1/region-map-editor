#pragma once

#include "data/Region.h"

struct SelectionState
{
    Region* selectedRegion = nullptr; // nullptr = intet valgt
    bool    popupOpen      = false;

    void select(Region* region)
    {
        selectedRegion = region;
        popupOpen      = (region != nullptr);
    }

    void clear()
    {
        selectedRegion = nullptr;
        popupOpen      = false;
    }

    bool hasSelection() const { return selectedRegion != nullptr; }
};
