#pragma once

#include "rendering/Camera.h"

class GridRenderer {
public:
    void render(const Camera& camera) const;

private:
    int gridHalfSize = 50;
    double gridSpacing = 50.0;
};