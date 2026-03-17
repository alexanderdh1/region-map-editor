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

    void toggleGrid() { showGrid = !showGrid; }
    bool isGridVisible() const { return showGrid; }

private:
    GridRenderer gridRenderer;
    TileLayer tileLayer;
    bool showGrid = false;
};