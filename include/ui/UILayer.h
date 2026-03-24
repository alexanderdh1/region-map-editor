#pragma once

// UILayer is the single entry point for all UI rendering and UI input handling.
//
// Design contract:
//   - UILayer reads state from Core (selection, input mode, region data)
//   - UILayer may WRITE back to Core (change region name, status, colour, delete)
//   - UILayer never touches Camera world-space coordinates directly
//   - When ImGui is introduced, only UILayer and its sub-panels need to change.

#include <vector>
#include "core/Core.h"
#include "math/Vec2.h"

class UILayer
{
public:
    void render(Core& core);
    bool onKeyPress(int glfwKey, Core& core);

    // Call from WindowCallbacks on left mouse press.
    // Returns true if UI consumed the click (map should not process it).
    bool onMouseClick(const Vec2& screenPos, Core& core);

private:
    void renderPopup(Core& core);
    void renderContextMenu(Core& core);
    void renderToolIndicator(const Core& core);

    // Click zones registered during renderPopup for sub-region navigation
    struct ClickZone {
        double x, y, w, h;
        int childIndex; // index into selectedRegion->children
    };
    std::vector<ClickZone> subRegionZones_;
};
