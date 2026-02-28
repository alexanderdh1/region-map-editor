#include <GLFW/glfw3.h>

#include "core/Core.h"
#include "rendering/Renderer.h"
#include "window/WindowCallbacks.h"
#include "window/WindowContext.h"
#include "window/WindowSetup.h"
#include "window/WindowUI.h"
#include "window/WindowFactory.h"
#include "window/OpenGLSetup.h"
#include "data/WorldLoader.h"

int main()
{
    GLFWwindow* window =
        createWindow(1280, 720, "Spatial Map Editor");

    if (!window)
        return -1;

    setupOpenGLState();

    Core core;
    Renderer renderer;

    // Load world
    loadSingleImageWorld(
        "assets/overworld",
        core,
        renderer
    );

    // Window bindings
    WindowContext context{ &core };
    setupWindowCallbacks(window, &context);
    setupInitialViewport(window, core);

    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);

    // -------- Main Loop --------
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