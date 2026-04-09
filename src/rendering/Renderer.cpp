#include "rendering/Renderer.h"
#include "core/Core.h"

void Renderer::render(const Core& core)
{
    const Camera& camera = core.getCamera();

    // 1. Map tiles
    tileLayer.render(camera);

    // 2. Regions
    regionRenderer.render(core.getRegionTree(), camera);

    // 3. Edit handles (drawn on top of regions, below UI)
    regionRenderer.renderEditHandles(core);

    // 4. Live drawing preview
    regionRenderer.renderPreview(core.getInput(), camera);
}

TileLayer& Renderer::getTileLayer()
{
    return tileLayer;
}
