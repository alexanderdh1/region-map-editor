#pragma once

#include "rendering/Camera.h"
#include "input/Input.h"
#include "math/BlockCoord.h"
#include "data/RegionTree.h"
#include "core/SelectionState.h"

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

    void setWorldBlockBounds(
        int minBlockX,
        int minBlockZ,
        int maxBlockX,
        int maxBlockZ
    );

    Vec2 blockToWorld(const BlockCoord& block) const;
    BlockCoord worldToBlock(const Vec2& worldPos) const;

    RegionTree& getRegionTree()             { return regionTree; }
    const RegionTree& getRegionTree() const { return regionTree; }

    SelectionState& getSelection()             { return selection; }
    const SelectionState& getSelection() const { return selection; }

private:
    Camera         camera;
    Input          input;
    RegionTree     regionTree;
    SelectionState selection;

    double worldWidth  = 0.0;
    double worldHeight = 0.0;

    int worldMinBlockX = 0;
    int worldMinBlockZ = 0;
    int worldMaxBlockX = 0;
    int worldMaxBlockZ = 0;
};