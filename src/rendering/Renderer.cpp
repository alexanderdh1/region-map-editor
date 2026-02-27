#include "rendering/Renderer.h"
#include "core/Core.h"
#include <iostream>

void Renderer::render(const Core& core)
{
    const Camera& camera = core.getCamera();

    tileLayer.render(camera);
    gridRenderer.render(camera);
}

TileLayer& Renderer::getTileLayer()
{
    return tileLayer;
}