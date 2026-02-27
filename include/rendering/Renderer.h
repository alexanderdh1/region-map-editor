#pragma once

#include "rendering/GridRenderer.h"
#include "rendering/TileLayer.h"

class Core;

class Renderer
{
public:
    Renderer() = default;

    void render(const Core& core);

    TileLayer& getTileLayer();


private:
    GridRenderer gridRenderer;
    TileLayer tileLayer;
};