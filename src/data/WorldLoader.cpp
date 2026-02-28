#include "data/WorldLoader.h"

#include "rendering/Texture.h"
#include "rendering/Tile.h"
#include "rendering/Renderer.h"
#include "core/Core.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

void loadSingleImageWorld(
    const std::string& basePath,
    Core& core,
    Renderer& renderer
)
{
    // -------------------------
    // Load texture
    // -------------------------

    static Texture sharedTexture(basePath + ".png");

    if (sharedTexture.getWidth() == 0 ||
        sharedTexture.getHeight() == 0)
    {
        throw std::runtime_error("Failed to load PNG: " + basePath);
    }

    // -------------------------
    // Load metadata JSON
    // -------------------------

    std::ifstream file(basePath + ".json");
    if (!file)
    {
        throw std::runtime_error(
            "Could not open metadata JSON: " + basePath + ".json"
        );
    }

    json j;
    file >> j;

    int minBlockX = j.at("minBlockX").get<int>();
    int minBlockZ = j.at("minBlockZ").get<int>();
    int maxBlockX = j.at("maxBlockX").get<int>();
    int maxBlockZ = j.at("maxBlockZ").get<int>();

    // -------------------------
    // Configure Core world data
    // -------------------------

    core.setWorldSize(
        sharedTexture.getWidth(),
        sharedTexture.getHeight()
    );

    core.setWorldBlockBounds(
        minBlockX,
        minBlockZ,
        maxBlockX,
        maxBlockZ
    );

    // -------------------------
    // Create single tile
    // -------------------------

    Tile tile;
    tile.texture = &sharedTexture;

    // Center image at world (0,0)
    tile.position = {
        -sharedTexture.getWidth()  / 2.0,
        -sharedTexture.getHeight() / 2.0
    };

    tile.size = {
        static_cast<double>(sharedTexture.getWidth()),
        static_cast<double>(sharedTexture.getHeight())
    };

    renderer.getTileLayer().addTile(std::move(tile));

    // -------------------------
    // Debug output
    // -------------------------

    std::cout << "World loaded:\n";
    std::cout << "Blocks from ("
              << minBlockX << ", "
              << minBlockZ << ") to ("
              << maxBlockX << ", "
              << maxBlockZ << ")\n";

    std::cout << "Image size: "
              << sharedTexture.getWidth()
              << " x "
              << sharedTexture.getHeight()
              << "\n";
}