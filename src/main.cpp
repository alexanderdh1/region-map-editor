#include <iostream>
#include <GLFW/glfw3.h>
#include <sstream>


#include "core/Core.h"
#include "input/Input.h"
#include "math/Vec2.h"

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "Spatial Map Editor",
        nullptr,
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    Core core;
    Input input;

    glfwSetWindowUserPointer(window, &input);

    glfwSetMouseButtonCallback(
        window,
        [](GLFWwindow* window, int button, int action, int /*mods*/) {
            if (button != GLFW_MOUSE_BUTTON_LEFT) return;

            auto* input =
                static_cast<Input*>(glfwGetWindowUserPointer(window));

            double x, y;
            glfwGetCursorPos(window, &x, &y);

            input->onMouseButton(action == GLFW_PRESS, Vec2{ x, y });
        }
    );

    glfwSetCursorPosCallback(
        window,
        [](GLFWwindow* window, double x, double y) {
            auto* input =
                static_cast<Input*>(glfwGetWindowUserPointer(window));

            input->onMouseMove(Vec2{ x, y });
        }
    );

    glfwSetScrollCallback(
    window,
    [](GLFWwindow* window, double /*xOffset*/, double yOffset) {
        auto* input =
            static_cast<Input*>(glfwGetWindowUserPointer(window));

        input->onScroll(yOffset);
    }
    );

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (input.hasPanDelta()) {
            Vec2 delta = input.consumePanDelta();
            core.getCamera().panBy(delta);

            std::cout
                << "Pan delta: "
                << delta.x << ", "
                << delta.y << "\n";
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);

        const Vec2& camPos = core.getCamera().position;

        std::ostringstream title;
        title << "Spatial Map Editor | World: ("
            << camPos.x << ", "
            << camPos.y << ")";

        glfwSetWindowTitle(window, title.str().c_str());
        if (input.hasZoomDelta()) {
            double zoomSteps = input.consumeZoomDelta();

            // Simple, kontrollerbar faktor
            const double zoomFactor = 1.1;

            if (zoomSteps > 0) {
                core.getCamera().zoomBy(zoomFactor);
                std::cout << "Zoom now: "
                    << core.getCamera().zoom
                    << "\n";
            } else if (zoomSteps < 0) {
                core.getCamera().zoomBy(1.0 / zoomFactor);
                std::cout << "Zoom now: "
                << core.getCamera().zoom
                << "\n";
            }
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
