#include <GLFW/glfw3.h>

#include "core/Core.h"
#include "rendering/Renderer.h"
#include "rendering/Tile.h"
#include "window/WindowCallbacks.h"
#include "window/WindowContext.h"
#include "window/WindowSetup.h"
#include "window/WindowUI.h"
#include "window/WindowFactory.h"

int main()
{
    GLFWwindow* window =
        createWindow(1280, 720, "Spatial Map Editor");

    if (!window)
        return -1;

    // --- OpenGL state ---
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Core core;
    Renderer renderer;

    // --- Add test tile ---
    // --- Load single Minecraft world image ---
    Texture sharedTexture("assets/overworld.png");

    core.setWorldSize(
    sharedTexture.getWidth(),
    sharedTexture.getHeight()
    );
    
    // --- Create single tile covering entire image ---
    Tile tile;

    tile.texture = &sharedTexture;

    tile.position = {
        -sharedTexture.getWidth() / 2.0,
        -sharedTexture.getHeight() / 2.0
    };

    tile.size = {
        static_cast<double>(sharedTexture.getWidth()),
        static_cast<double>(sharedTexture.getHeight())
    };

renderer.getTileLayer().addTile(std::move(tile));

    // --- Window bindings ---
    WindowContext context{ &core };
    setupWindowCallbacks(window, &context);
    setupInitialViewport(window, core);

    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);

    // =========================
    // ======= MAIN LOOP ======
    // =========================

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        core.update(window);

        glClear(GL_COLOR_BUFFER_BIT);

        renderer.render(core);

        glfwSwapBuffers(window);

        updateWindowTitle(window, core);
    }

    destroyWindow(window);
    return 0;
}