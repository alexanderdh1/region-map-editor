#pragma once

// UILayer is the single entry point for all UI rendering and UI input handling.
//
// Design contract:
//   - UILayer reads state from Core (selection, input mode, region data)
//   - UILayer may WRITE back to Core (change region name, status, colour, delete)
//   - UILayer never touches Camera world-space coordinates directly —
//     that belongs to rendering/ and Core
//   - When ImGui is introduced, only UILayer and its sub-panels need to change.
//     The rest of the codebase (rendering, core, data, input) stays untouched.

#include "core/Core.h"

class UILayer
{
public:
    // Call once per frame after map rendering.
    // Handles both drawing UI and processing UI keyboard input.
    void render(Core& core);

    // Call from WindowCallbacks when a key is pressed.
    // Returns true if the UI consumed the event (so Core should not also handle it).
    bool onKeyPress(int glfwKey, Core& core);

private:
    void renderPopup(Core& core);
    void renderToolIndicator(const Core& core);
};
