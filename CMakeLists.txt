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
)

# Include directories
target_include_directories(main PRIVATE include)

# GLFW
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(main PRIVATE glfw opengl32)
