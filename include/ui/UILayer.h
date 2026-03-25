#pragma once

// UILayer is the single entry point for all UI rendering and UI input handling.
//
// Design contract:
//   - UILayer reads state from Core (selection, input mode, region data)
//   - UILayer may WRITE back to Core (change region name, status, colour, delete)
//   - UILayer never touches Camera world-space coordinates directly
//   - When ImGui is introduced, only UILayer and its sub-panels need to change.

#include <vector>
#include <string>
#include "core/Core.h"
#include "math/Vec2.h"

class UILayer
{
public:
    void render(Core& core);
    bool onKeyPress(int glfwKey, Core& core);
    bool onMouseClick(const Vec2& screenPos, Core& core);

    // Called from WindowCallbacks for printable character input
    void onCharInput(unsigned int codepoint, Core& core);

    // True when a text field is active — blocks map shortcuts
    bool isTextInputActive() const { return activeField_ != Field::None; }

private:
    void renderPopup(Core& core);
    void renderContextMenu(Core& core);
    void renderToolIndicator(const Core& core);

    // Draw a text field — returns true if it was clicked
    bool renderTextField(
        double x, double y, double w, double h,
        const std::string& value,
        bool active,
        const std::string& placeholder = "");

    // Text input state
    enum class Field { None, Name, Note };
    Field        activeField_  = Field::None;
    std::string  editBuffer_;   // current edited text
    int          cursorPos_  = 0;
    double       cursorBlink_ = 0.0; // time accumulator for blink

    // Click zones for sub-region navigation and text fields
    struct ClickZone {
        double x, y, w, h;
        int childIndex; // -1 = context menu action, -2 = name field, -3 = note field
    };
    std::vector<ClickZone> subRegionZones_;
};
