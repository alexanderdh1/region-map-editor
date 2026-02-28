#pragma once

#include <string>

class Core;
class Renderer;

void loadSingleImageWorld(
    const std::string& path,
    Core& core,
    Renderer& renderer
);