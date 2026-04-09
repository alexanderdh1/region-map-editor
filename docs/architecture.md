# Architecture

This document describes the software architecture of the Spatial Map Editor. The goal is to keep the system understandable, modular, and easy to extend, while remaining close to what is actually implemented in code.

The architecture is intentionally kept simple and explicit. Every part of the system has a clear responsibility, and communication between parts is tightly controlled.

---

## 1. Architectural Goals

- Clear separation of responsibilities
- Predictable data flow
- Low coupling between subsystems
- Easy extension without breaking existing code
- Good performance for large spatial datasets

The system favors clarity over abstraction. If a concept is hard to explain, it does not belong in the architecture.

---

## 2. System Overview

### 2.1 Purpose

The Spatial Map Editor is a desktop application for visualizing and organizing large-scale spatial data. Users load a map image, navigate it freely, draw regions of interest on top of it, and annotate those regions with names, notes, and colours.

Regions can be nested hierarchically and are serialized to a local JSON file. The system supports two coordinate modes: normalised image coordinates (0.0–1.0) for generic map images, and block coordinates for images with accompanying spatial metadata.

### 2.2 High-level data flow

1. **Window** receives raw OS events (mouse, keyboard, resize) and forwards them to ImGui and the Input module.
2. **Input** translates raw events into typed actions (pan delta, zoom delta, draw start, edit drag, etc.) and stores them as pending state.
3. **Core** consumes pending input state each frame, updates application state, and coordinates all modules.
4. **Data** is read and written by Core — region geometry, hierarchy, and metadata live here.
5. **Rendering** reads Core and Data state and draws the current frame — it does not write state.
6. **UI** reads Core and Data state to draw ImGui panels, and calls back into Core when the user performs an action.

### 2.3 Module diagram

```
             +--------+
             |  User  |
             +--------+
                 |
                 v
        +------------------+
        |      Window      |
        +------------------+
                 |
                 v
        +------------------+
        |      Input       |
        +------------------+
                 |
                 v
+-------+    +----------+    +-----------+
|  UI   | <> |   Core   | -> | Rendering |
+-------+    +----------+    +-----------+
                 |
                 v
        +------------------+
        |       Data       |
        +------------------+
```

---

## 3. Module Breakdown

### 3.1 Core (`src/core/`)

The central coordinator of the system. Owns the main application state and drives the update loop each frame.

Responsibilities:
- Owns `Camera`, `Input`, `RegionTree`, `SelectionState`, and `EditState`
- Processes pending input to update camera position, zoom, and draw state
- Handles region creation (rectangle and polygon) including parent assignment and minimum size validation
- Drives edit mode: handle detection, absolute-positioned drag, constraint checking against parent and children
- Dispatches save calls to `RegionSerializer` after structural changes

Core is the only module allowed to coordinate interactions between other modules.

### 3.2 Data (`src/data/`)

Owns and manages all persistent application state.

Responsibilities:
- `Region` — geometry, colour, name, note, hidden/collapsed flags, parent pointer, and owned children
- `RegionGeometry` — rectangle and polygon geometry with winding-number point containment and self-intersection detection
- `RegionTree` — owns all top-level regions, supports add, remove, move (reparent), and depth-first traversal
- `RegionSerializer` — saves and loads the region tree to/from JSON; supports normalised and block coordinate modes; auto-detects mode mismatch on load
- `WorldLoader` — loads the map image, detects coordinate mode from the presence of a `.json` metadata file, and sets world bounds on Core

State is modified exclusively through Core. Other modules read data but do not write it directly.

### 3.3 Input (`src/input/`)

Translates raw GLFW events into typed, consumable actions for Core.

Responsibilities:
- Tracks mouse button state, drag deltas, and scroll events
- Manages draw tool state (`Navigate`, `Rectangle`, `Polygon`, `Edit`)
- Accumulates pan delta for navigation
- Tracks edit drag with both per-frame delta and total-since-start delta (used for absolute constraint-safe positioning)
- Detects polygon close (click near first point) and double-click polygon completion
- Exposes pending actions as boolean flags with consume methods — Core clears them each frame

Input does not modify application state directly. It only records user intent.

### 3.4 Rendering (`src/rendering/`)

Visualizes current application state using OpenGL fixed-function pipeline. Purely read-only with respect to state.

Responsibilities:
- `Camera` — stores position, zoom, and viewport size; converts between world and screen space; clamps position to world bounds
- `TileLayer` / `Tile` / `Texture` — loads and draws the map image as a textured quad
- `RegionRenderer` — draws filled polygons using a stencil-based even-odd fill (GL_INVERT), outlines, and edit handles; skips hidden regions recursively
- `GridRenderer` — optional grid overlay
- `Renderer` — top-level render coordinator; calls tile, region, and edit handle rendering in order

Rendering never makes decisions or modifies state. It draws exactly what Core and Data describe.

### 3.5 UI (`src/ui/`)

Provides the ImGui-based graphical interface. Reads state from Core and Data, and calls back into Core when the user performs an action.

Responsibilities:
- **Left sidebar** — collapsible region tree with visibility toggles, collapse arrows, selection, and drag-and-drop reparenting (deferred execution to avoid iterator invalidation)
- **Region popup** — name and note editing with debounced auto-save, colour picker, edit mode toggle, sub-region creation, and delete
- **Right sidebar** — rectangle and polygon tool buttons
- Handles all keyboard shortcuts and blocks them when a text field has focus

UI never modifies `RegionTree` or geometry directly. All structural changes go through Core or `RegionSerializer`.

### 3.6 Window (`src/window/`)

Handles GLFW lifecycle, OpenGL initialization, and event forwarding. Acts as the bridge between the OS and the rest of the system.

Responsibilities:
- `WindowFactory` — initializes GLFW, creates the window with a stencil buffer, and sets the OpenGL context
- `OpenGLSetup` — configures initial OpenGL state (blending, texture, stencil clear)
- `WindowSetup` — sets the initial viewport and projection matrix from the framebuffer size
- `WindowCallbacks` — registers GLFW callbacks for resize, mouse buttons, mouse move, scroll, keyboard, and character input; routes events to ImGui first, then to Input and UI as appropriate
- `WindowUI` — updates the window title each frame with camera position, cursor world coordinates, and zoom level; manages the crosshair cursor when hovering near edit handles

---

## 4. State Ownership

| State | Owner |
|---|---|
| Camera position and zoom | `Core` (via `Camera`) |
| Current draw tool and input events | `Core` (via `Input`) |
| Selected region and view stack | `Core` (via `SelectionState`) |
| Active edit target and handle | `Core` (via `EditState`) |
| Region tree and all region data | `Core` (via `RegionTree`) |
| Coordinate mode and world bounds | `Core` |

All persistent state is serialized by `RegionSerializer` to `regions.json`.

---

## 5. Coordinate Systems

The system uses three coordinate spaces:

- **Screen space** — pixels from top-left of the window
- **World space** — centered at (0, 0), Y+ is up, units are image pixels
- **Serialized coordinates** — either normalised (0.0–1.0 relative to image size) or block integers (when a `.json` metadata file is present)

`Camera` converts between screen and world space. `RegionSerializer` converts between world and serialized space on save/load.

---

## 6. Scope and Exclusions

The following are explicitly out of scope:

- Multiplayer or collaborative editing
- Real-time synchronization or networking
- Simulation or game-engine behavior
- Undo/redo (planned for a future version)
