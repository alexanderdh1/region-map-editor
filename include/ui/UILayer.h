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
    void renderSidebar(Core& core);       // right sidebar (tools)
    void renderRegionTree(Core& core);    // left sidebar (region tree)
    void renderPopup(Core& core);
    void renderContextMenu(Core& core);

    // Left sidebar state
    bool treeExpanded_ = false;

    // ImGui text field focus tracking
    bool nameFieldActive_ = false;
    bool noteFieldActive_ = false;
};
