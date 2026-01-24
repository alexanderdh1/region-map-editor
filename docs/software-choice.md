# Software Choice and Platform Decisions
This document describes the core software and platform decisions for the Spatial Map Editor project. The purpose is to formally document early technical choices in order to provide a stable foundation for architecture, implementation, and future extensions.

These decisions are considered fixed unless explicitly revised in later design iterations.

## 1. Project Scope

The system is designed as an interactive desktop application for visualizing and editing very large-scale spatial data. The primary technical challenges addressed by the project are:

- efficient rendering of large, grid-based worlds

- precise spatial interaction (pan, zoom, coordinate mapping)

- support for free-form polygon regions

- hierarchical metadata and progress tracking

The system is designed to be domain-agnostic, meaning it is not tied to a specific data source or application domain.

## 2. Platform Decision
### Chosen Platform: Local Desktop Application
The project is implemented as a local desktop application, rather than a web-based solution, to ensure full control over performance, memory usage, and offline data access.

## 3. Programming Language
C++ is used as the primary implementation language across all core modules.

## 4. Graphics and Rendering
### Chosen Technology: OpenGL

- Cross-platform, low-level graphics API.

- Suitable for 2D rendering with precise control over transformations.

- Supports efficient implementations of pan, zoom, tiling, and overlays.

- Allows future optimization strategies such as batching, level-of-detail, and instancing.

The rendering layer is treated as a standalone subsystem, isolated from input handling and data management.

## 5. Windowing, Input, and UI
### Window and Input Handling

- A lightweight windowing library (e.g. GLFW or SDL) is used for:

  - window creation

  - input events (mouse and keyboard)

This ensures portability while keeping external dependencies minimal.

### User Interface

- UI elements such as side panels, text fields, and progress bars are implemented using an immediate-mode UI approach.

- The UI layer acts as a control and visualization layer on top of the rendered map.

UI logic is strictly separated from rendering and data representation.

## 6. Architectural Principles

The following architectural principles are enforced from the start:

- Clear separation of concerns between:

    - core application logic

    - rendering

    - input handling

    - data management

    - user interface

- Data models must be independent of rendering and UI.

- Rendering must be stateless with respect to user input.

- All cross-module coordination is handled centrally.

These principles ensure maintainability, testability, and long-term extensibility.