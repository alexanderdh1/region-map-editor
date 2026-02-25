#include "rendering/Renderer.h"
#include "core/Core.h"

void Renderer::render(const Core& core)
{
    gridRenderer.render(core.getCamera());
}