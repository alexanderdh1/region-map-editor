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

    bool treeExpanded_    = false;
    bool nameFieldActive_ = false;
    bool noteFieldActive_ = false;

    // Debounce auto-save for name/note text fields.
    // When either field is edited, dirtyTime_ is set to glfwGetTime().
    // If it stays dirty for SAVE_DEBOUNCE_S seconds, we save.
    double dirtyTime_   = -1.0;  // -1 = not dirty
    static constexpr double SAVE_DEBOUNCE_S = 1.0;
};
