# Software Choice and Platform Decisions

This document describes the core software and platform decisions for the Spatial Map Editor. The purpose is to formally document technical choices in order to provide a stable foundation for architecture, implementation, and future extensions.

---

## 1. Project Scope

The system is designed as an interactive desktop application for visualizing and editing spatial data on top of a map image. The primary technical challenges addressed are:

- Efficient rendering of large map images
- Precise spatial interaction (pan, zoom, coordinate mapping)
- Support for free-form polygon and rectangle regions with hierarchical organization

The system is domain-agnostic and is not tied to a specific data source or application domain.

---

## 2. Platform

**Local desktop application.**

Chosen over a web-based solution to ensure full control over performance, memory usage, and offline data access. No network dependency of any kind.

---

## 3. Programming Language

**C++17/20.**

Used across all modules. Chosen for performance, direct OpenGL access, and control over memory layout. `std::unique_ptr` ownership is used throughout to make data ownership explicit.

---

## 4. Graphics and Rendering

**OpenGL (legacy fixed-function pipeline).**

- Suitable for 2D rendering with precise control over transformations
- Stencil buffer used for even-odd polygon fill (GL_INVERT)
- Fixed-function chosen over shaders to minimize complexity for a 2D editor

The rendering layer is a standalone subsystem isolated from input handling and data management.

---

## 5. Windowing and Input

**GLFW3.**

- Lightweight, cross-platform windowing and input library
- Handles window creation, framebuffer resize, mouse, keyboard, and scroll events
- Event callbacks route to ImGui first, then to the Input module

---

## 6. User Interface

**Dear ImGui (immediate-mode UI, OpenGL2 backend).**

- Panels, text fields, buttons, colour picker, and drag-and-drop tree implemented with ImGui
- Immediate-mode approach fits the per-frame update model naturally
- UI logic is strictly separated from rendering and data — the UI layer only calls back into Core

---

## 7. Serialization

**nlohmann/json.**

- Region tree is serialized to a local `regions.json` file
- Supports two coordinate modes: normalised (0.0–1.0) and block integers
- Mode is auto-detected at load time from the presence of a spatial metadata file
- Auto-save triggers on every structural change; name/note edits are debounced by 1 second

---

## 8. Image Loading

**stb_image (bundled).**

- Single-header PNG loader
- Used to load the map image into an OpenGL texture

---

## 9. Build System

**CMake + Ninja on MinGW (MSYS2 MINGW64) on Windows.**

- `vcpkg` used for GLFW dependency
- ImGui, nlohmann/json, and stb_image are bundled in `external/`

---

## 10. Architectural Principles

The following principles are enforced throughout:

- Clear separation between core logic, rendering, input, data, UI, and window management
- Data models are independent of rendering and UI
- Rendering is stateless with respect to user input — it only reads state, never writes it
- All cross-module coordination is handled exclusively by Core
- Input records user intent as consumable flags — it does not modify application state directly
