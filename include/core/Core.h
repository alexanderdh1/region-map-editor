#pragma once

#include "rendering/Camera.h"
#include "input/Input.h"

struct GLFWwindow;

class Core {
public:
    Core();

    Camera& getCamera();
    const Camera& getCamera() const;

    Input& getInput();            
    const Input& getInput() const; 

    void update(GLFWwindow* window); 

    void setWorldSize(double width, double height);

private:
    Camera camera;
    Input input; 

    double worldWidth = 0.0;
    double worldHeight = 0.0;
};