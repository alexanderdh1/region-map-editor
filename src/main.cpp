#include <GLFW/glfw3.h>

#include "core/Core.h"
#include "rendering/Renderer.h"
#include "window/WindowCallbacks.h"
#include "window/WindowContext.h"
#include "window/WindowSetup.h"
#include "window/WindowUI.h"
#include "window/WindowFactory.h"

int main()
{
    
    GLFWwindow* window = createWindow(1280, 720, "Spatial Map Editor");

    if (!window)
        return -1;

    Core core;
    Renderer renderer;

    // Bind Core to window callbacks
    WindowContext context{ &core };
    setupWindowCallbacks(window, &context);

    // ---- Initial viewport setup ----
    setupInitialViewport(window,core);

    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);

    // =========================
    // ======= MAIN LOOP ======
    // =========================

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // All engine logic (pan + pivot zoom)
        core.update(window);

        glClear(GL_COLOR_BUFFER_BIT);

        renderer.render(core);

        glfwSwapBuffers(window);

        updateWindowTitle(window, core);
    }

    destroyWindow(window);
    return 0;
}