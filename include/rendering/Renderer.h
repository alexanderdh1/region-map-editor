#pragma once

#include "rendering/GridRenderer.h"
#include "rendering/RegionRenderer.h"
#include "rendering/PopupRenderer.h"
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
    GridRenderer   gridRenderer;
    RegionRenderer regionRenderer;
    PopupRenderer  popupRenderer;
    TileLayer      tileLayer;
    bool           showGrid = false;
};