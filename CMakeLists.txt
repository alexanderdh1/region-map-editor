cmake_minimum_required(VERSION 3.21)

project(SpatialMapEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Executable
add_executable(main
    src/main.cpp
    src/core/Core.cpp
    src/input/Input.cpp
    src/rendering/Camera.cpp
    src/rendering/GridRenderer.cpp
    src/rendering/RegionRenderer.cpp
    src/rendering/PopupRenderer.cpp
    src/window/WindowCallbacks.cpp
    src/window/WindowSetup.cpp
    src/window/WindowUI.cpp
    src/window/WindowFactory.cpp
    src/rendering/Renderer.cpp
    src/rendering/Texture.cpp
    src/rendering/TileLayer.cpp
    src/data/WorldLoader.cpp
    src/data/RegionTree.cpp
    src/window/OpenGLSetup.cpp
)

# Include directories
target_include_directories(main PRIVATE 
include
external
)

# GLFW
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(main PRIVATE glfw opengl32)
