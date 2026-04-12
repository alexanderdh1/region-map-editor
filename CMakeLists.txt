cmake_minimum_required(VERSION 3.21)

project(RegionMapEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ImGui sources
set(IMGUI_DIR external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl2.cpp
)

# Executable
add_executable(main
    src/main.cpp
    src/core/Core.cpp
    src/core/HistoryManager.cpp
    src/input/Input.cpp
    src/rendering/Camera.cpp
    src/rendering/RegionRenderer.cpp
    src/window/WindowCallbacks.cpp
    src/window/WindowSetup.cpp
    src/window/WindowUI.cpp
    src/window/WindowFactory.cpp
    src/rendering/Renderer.cpp
    src/rendering/Texture.cpp
    src/rendering/TileLayer.cpp
    src/data/MapLoader.cpp
    src/data/RegionTree.cpp
    src/data/RegionSerializer.cpp
    src/window/OpenGLSetup.cpp
    src/ui/UILayer.cpp
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(main PRIVATE
    include
    external
    ${IMGUI_DIR}
)

# GLFW
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(main PRIVATE glfw opengl32 stdc++fs)
