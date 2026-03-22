#pragma once

#include "core/SelectionState.h"
#include "input/Input.h"
#include "rendering/Camera.h"

class PopupRenderer
{
public:
    void render(const SelectionState& selection, const Camera& camera) const;

    // Small tool indicator — bottom-left, uses viewport height to stay there
    void renderToolIndicator(const Input& input, const Camera& camera) const;

private:
    void drawPanel(double x, double y, double w, double h,
                   float r, float g, float b, float a) const;

    void drawText(double x, double y, const std::string& text,
                  float r, float g, float b) const;
};
