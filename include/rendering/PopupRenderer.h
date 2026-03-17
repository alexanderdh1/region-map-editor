#pragma once

#include "core/SelectionState.h"
#include "rendering/Camera.h"

class PopupRenderer
{
public:
    // Draw the popup for the currently selected region.
    // Returns true if the popup consumed the frame (i.e. is visible).
    void render(const SelectionState& selection, const Camera& camera) const;

private:
    void drawPanel(double x, double y, double w, double h,
                   float r, float g, float b, float a) const;

    void drawText(double x, double y, const std::string& text,
                  float r, float g, float b) const;
};
