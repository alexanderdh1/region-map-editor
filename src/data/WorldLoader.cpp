#include "data/WorldLoader.h"

#include "rendering/Texture.h"
#include "rendering/Tile.h"
#include "core/Core.h"
#include "rendering/Renderer.h"

void loadSingleImageWorld(
    const std::string& path,
    Core& core,
    Renderer& renderer
)
{
    static Texture sharedTexture(path);

    core.setWorldSize(
        sharedTexture.getWidth(),
        sharedTexture.getHeight()
    );

    Tile tile;
    tile.texture = &sharedTexture;

    tile.position = {
        -sharedTexture.getWidth() / 2.0,
        -sharedTexture.getHeight() / 2.0
    };

    tile.size = {
        (double)sharedTexture.getWidth(),
        (double)sharedTexture.getHeight()
    };

    renderer.getTileLayer().addTile(std::move(tile));
}