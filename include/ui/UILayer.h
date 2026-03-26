#pragma once

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
    void onCharInput(unsigned int codepoint, Core& core);

    bool isTextInputActive() const { return nameFieldActive_ || noteFieldActive_; }

private:
    void renderSidebar(Core& core);
    void renderPopup(Core& core);
    void renderContextMenu(Core& core);

    // ImGui text field focus tracking
    bool nameFieldActive_ = false;
    bool noteFieldActive_ = false;

    // Legacy bitmap UI state (kept for onMouseClick click zone detection)
    struct ClickZone {
        double x, y, w, h;
        int childIndex;
    };
    std::vector<ClickZone> subRegionZones_;
};
