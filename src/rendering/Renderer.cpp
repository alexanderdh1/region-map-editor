#include "rendering/Renderer.h"
#include "core/Core.h"

void Renderer::render(const Core& core)
{
    const Camera& camera = core.getCamera();

    // 1. Map tiles (background)
    tileLayer.render(camera);

    // 2. Regions (on top of map)
    regionRenderer.render(core.getRegionTree(), camera);

    // 3. Live rectangle preview while drawing
    regionRenderer.renderPreview(core.getInput(), camera);

    // 4. Selection popup (on top of everything)
    popupRenderer.render(core.getSelection(), camera);

    // 5. Debug grid (optional)
    if (showGrid)
        gridRenderer.render(camera);
}

TileLayer& Renderer::getTileLayer()
{
    return tileLayer;
}