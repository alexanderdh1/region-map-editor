#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

#include "core/Core.h"
#include "rendering/Renderer.h"
#include "ui/UILayer.h"
#include "window/WindowCallbacks.h"
#include "window/WindowContext.h"
#include "window/WindowSetup.h"
#include "window/WindowUI.h"
#include "window/WindowFactory.h"
#include "window/OpenGLSetup.h"
#include "data/MapLoader.h"
#include "data/RegionSerializer.h"

#include <iostream>
#include <stdexcept>
#include <filesystem>

int main()
{
    GLFWwindow* window = createWindow(1280, 720, "Spatial Map Editor");
    if (!window) return -1;

    setupOpenGLState();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    io.Fonts->AddFontFromFileTTF("assets/fonts/JetBrainsMono-Regular.ttf", 16.0f);
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL2_Init();

    Core     core;
    Renderer renderer;
    UILayer  uiLayer;

    try
    {
        loadMap("assets/map", core, renderer);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Error] Failed to load world: " << e.what() << "\n";
        destroyWindow(window);
        return -1;
    }

    // Auto-load regions — world mode is now set, coordinates convert correctly
    if (std::filesystem::exists("regions.json"))
        RegionSerializer::load(core.getRegionTree(), "regions.json", core);

    WindowContext context{ &core, &uiLayer };
    setupWindowCallbacks(window, &context);
    setupInitialViewport(window, core);

    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        core.update(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        renderer.render(core);

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        uiLayer.render(core);

        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        updateWindowTitle(window, core);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    destroyWindow(window);
    return 0;
}
