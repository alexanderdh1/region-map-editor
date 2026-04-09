#pragma once

#include <string>

class Core;
class Renderer;

void loadMap(
    const std::string& path,
    Core& core,
    Renderer& renderer
);