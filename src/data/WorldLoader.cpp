#include "data/WorldLoader.h"

#include "rendering/Texture.h"
#include "rendering/Tile.h"
#include "rendering/Renderer.h"
#include "core/Core.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <filesystem>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

void loadSingleImageWorld(
    const std::string& basePath,
    Core& core,
    Renderer& renderer
)
{
    // ---- Load texture ----
    auto texture = std::make_shared<Texture>(basePath + ".png");
    if (!texture->isValid())
        throw std::runtime_error("Failed to load PNG: " + basePath + ".png");

    double imgW = static_cast<double>(texture->getWidth());
    double imgH = static_cast<double>(texture->getHeight());

    core.setWorldSize(imgW, imgH);

    // ---- Detect mode from JSON presence ----
    std::string jsonPath = basePath + ".json";
    bool hasJson = std::filesystem::exists(jsonPath);

    if (hasJson)
    {
        // Block-coordinate mode: metadata available
        std::ifstream file(jsonPath);
        json j;
        file >> j;

        int minBlockX = j.at("minBlockX").get<int>();
        int minBlockZ = j.at("minBlockZ").get<int>();
        int maxBlockX = j.at("maxBlockX").get<int>();
        int maxBlockZ = j.at("maxBlockZ").get<int>();

        core.setWorldBlockBounds(minBlockX, minBlockZ, maxBlockX, maxBlockZ);
        core.setBlockCoordMode(true);
    }
    else
    {
        // Image mode: no metadata, use normalised coordinates
        core.setBlockCoordMode(false);
    }

    // ---- Create single tile ----
    Tile tile;
    tile.texture  = texture;
    tile.position = { -imgW / 2.0, -imgH / 2.0 };
    tile.size     = { imgW, imgH };

    renderer.getTileLayer().addTile(std::move(tile));
}
