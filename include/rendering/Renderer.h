#pragma once

#include "rendering/GridRenderer.h"

class Core;

class Renderer
{
public:
    Renderer() = default;

    void render(const Core& core);

private:
    GridRenderer gridRenderer;
};