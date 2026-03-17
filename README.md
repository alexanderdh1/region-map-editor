# Spatial Map Editor

A desktop application for visualizing and navigating large-scale spatial maps. Built in C++ with OpenGL.

Pan and zoom smoothly across large map images with pixel-accurate block coordinate tracking.

---

## Features

- **Pan & zoom** — click-drag to pan, scroll to zoom toward cursor
- **World bounds clamping** — camera stays within the map at all zoom levels
- **Block coordinate mapping** — converts screen positions to world block coordinates (designed for Minecraft-style grids)
- **Tile-based rendering** — frustum-culled tile layer for efficient rendering
- **Modular architecture** — clean separation between core, input, rendering, and data

---

## Screenshots

> _Add a screenshot here once the application is running_

---

## Getting Started

### Prerequisites

- CMake 3.21+
- A C++20 compiler (MSVC, GCC, Clang)
- [vcpkg](https://github.com/microsoft/vcpkg) for dependency management

### Dependencies

| Library | Purpose |
|---|---|
| [GLFW](https://www.glfw.org/) | Window creation and input |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON metadata parsing |
| [stb_image](https://github.com/nothings/stb) | PNG loading (bundled) |

### Build

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/spatial-map-editor.git
cd spatial-map-editor

# Install dependencies via vcpkg
vcpkg install glfw3 nlohmann-json

# Configure and build
cmake -B build -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

### Assets

The application expects map assets at `assets/<name>.png` and `assets/<name>.json`.

The JSON metadata file describes the block coordinate bounds of the image:

```json
{
  "minBlockX": -2048,
  "minBlockZ": -2048,
  "maxBlockX":  2048,
  "maxBlockZ":  2048
}
```

---

## Project Structure

```
src/
├── core/        # Application lifecycle and update loop
├── data/        # World loading and asset management
├── input/       # Mouse and keyboard input handling
├── rendering/   # Camera, tile rendering, OpenGL drawing
└── window/      # GLFW window setup, callbacks, UI title

include/         # Header files mirroring src/ structure
external/        # Bundled third-party headers (stb, nlohmann)
docs/            # Architecture and design documentation
```

See [`docs/architecture.md`](docs/architecture.md) for a full breakdown of the system design.

---

## Controls

| Input | Action |
|---|---|
| Left mouse + drag | Pan the map |
| Scroll wheel | Zoom in/out toward cursor |

---

## Roadmap

- [ ] Region drawing (polygons)
- [ ] Markers with notes
- [ ] Side panel UI (ImGui)
- [ ] Save/load regions to JSON

---

## License

MIT
