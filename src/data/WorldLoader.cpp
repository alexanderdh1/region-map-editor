#include "data/WorldLoader.h"

#include "rendering/Texture.h"
#include "rendering/Tile.h"
#include "rendering/Renderer.h"
#include "core/Core.h"

#include <fstream>
#include <iostream>
#include <memory>
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

    // Texture ownership is transferred to TileLayer via shared_ptr,
    // so reloading with a different path works correctly.
    auto texture = std::make_shared<Texture>(basePath + ".png");

    if (!texture->isValid())
    {
        throw std::runtime_error("Failed to load PNG: " + basePath + ".png");
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
        texture->getWidth(),
        texture->getHeight()
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
    tile.texture = texture;

    // Center image at world (0,0)
    tile.position = {
        -texture->getWidth()  / 2.0,
        -texture->getHeight() / 2.0
    };

    tile.size = {
        static_cast<double>(texture->getWidth()),
        static_cast<double>(texture->getHeight())
    };

    renderer.getTileLayer().addTile(std::move(tile));

    std::cout << "[WorldLoader] Loaded: " << basePath << "\n"
              << "  Image : " << texture->getWidth()
              << " x "        << texture->getHeight() << " px\n"
              << "  Blocks: ("
              << minBlockX << ", " << minBlockZ << ") to ("
              << maxBlockX << ", " << maxBlockZ << ")\n";
}