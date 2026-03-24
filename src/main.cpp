#include <GLFW/glfw3.h>

#include "core/Core.h"
#include "rendering/Renderer.h"
#include "ui/UILayer.h"
#include "window/WindowCallbacks.h"
#include "window/WindowContext.h"
#include "window/WindowSetup.h"
#include "window/WindowUI.h"
#include "window/WindowFactory.h"
#include "window/OpenGLSetup.h"
#include "data/WorldLoader.h"
#include "data/RegionSerializer.h"

#include <iostream>
#include <stdexcept>
#include <filesystem>

int main()
{
    GLFWwindow* window =
        createWindow(1280, 720, "Spatial Map Editor");

    if (!window)
        return -1;

    setupOpenGLState();

    Core     core;
    Renderer renderer;
    UILayer  uiLayer;

    try
    {
        loadSingleImageWorld(
            "assets/overworld",
            core,
            renderer
        );
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Error] Failed to load world: " << e.what() << "\n";
        destroyWindow(window);
        return -1;
    }

    // Auto-load regions if save file exists
    if (std::filesystem::exists("regions.json"))
        RegionSerializer::load(core.getRegionTree(), "regions.json");

    WindowContext context{ &core, &uiLayer };
    setupWindowCallbacks(window, &context);
    setupInitialViewport(window, core);

    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);

    // -------- Main Loop --------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        core.update(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // 1. Map + regions (world-space)
        renderer.render(core);

        // 2. UI overlay (screen-space)
        uiLayer.render(core);

        glfwSwapBuffers(window);

        updateWindowTitle(window, core);
    }

    destroyWindow(window);
    return 0;
}