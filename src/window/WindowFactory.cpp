#include "window/WindowFactory.h"

#include <GLFW/glfw3.h>
#include <iostream>

GLFWwindow* createWindow(int width, int height, const char* title)
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    // Request 8-bit stencil buffer for polygon fill rendering
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    GLFWwindow* window = glfwCreateWindow(
        width,
        height,
        title,
        nullptr,
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    return window;
}

void destroyWindow(GLFWwindow* window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}