#pragma once
#include "rendering/Camera.h"
#include "input/Input.h"

class Core {
public:
    Core();

    void update();

    Camera& getCamera();
    const Camera& getCamera() const;

    Input& getInput();

private:
    Camera camera;
    Input input;
};
