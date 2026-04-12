#pragma once

#include "core/Core.h"
#include "math/Vec2.h"

class UILayer
{
public:
    void render(Core& core);
    bool onKeyPress(int glfwKey, int mods, Core& core);

    bool isTextInputActive() const { return nameFieldActive_ || noteFieldActive_; }

private:
    void renderSidebar(Core& core);
    void renderRegionTree(Core& core);
    void renderPopup(Core& core);

    bool treeExpanded_ = false;
    bool nameFieldActive_ = false;
    bool noteFieldActive_ = false;
    bool colourPickerOpen_ = false;

    // Debounce auto-save for name/note text fields
    double dirtyTime_ = -1.0;
    bool   snapshotPushedForDirty_ = false;
    static constexpr double SAVE_DEBOUNCE_S = 1.0;
};
