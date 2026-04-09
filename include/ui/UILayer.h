#pragma once

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
    void renderRegionTree(Core& core);
    void renderPopup(Core& core);

    bool treeExpanded_      = false;
    bool nameFieldActive_   = false;
    bool noteFieldActive_   = false;
    bool colourPickerOpen_  = false;

    // Debounce auto-save for name/note text fields
    double dirtyTime_ = -1.0;
    static constexpr double SAVE_DEBOUNCE_S = 1.0;

    // Tree drag-and-drop state
    RegionId dragSourceId_  = 0;     // region being dragged (0 = none)
    RegionId dropTargetId_  = 0;     // region hovered as potential parent (0 = root)
    bool     draggingInTree_ = false;
    Vec2     dragGhostPos_  { 0, 0 };
    std::string dragGhostName_;
};
