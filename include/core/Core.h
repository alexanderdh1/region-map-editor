#pragma once

#include "rendering/Camera.h"
#include "input/Input.h"
#include "math/BlockCoord.h"
#include "data/RegionTree.h"
#include "core/SelectionState.h"

struct GLFWwindow;

enum class EditHandleType
{
    None,
    RectCorner,
    PolyPoint,
};

struct EditState
{
    Region*        target      = nullptr;
    EditHandleType handleType  = EditHandleType::None;
    int            handleIndex = -1;
    Vec2 dragOriginWorld { 0.0, 0.0 };

    bool isActive()   const { return target != nullptr; }
    bool isDragging() const { return handleType != EditHandleType::None; }

    void clear()
    {
        target          = nullptr;
        handleType      = EditHandleType::None;
        handleIndex     = -1;
        dragOriginWorld = { 0.0, 0.0 };
    }

    void clearDrag()
    {
        handleType      = EditHandleType::None;
        handleIndex     = -1;
        dragOriginWorld = { 0.0, 0.0 };
    }
};

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
        int minBlockX, int minBlockZ,
        int maxBlockX, int maxBlockZ
    );

    // Called by WorldLoader to signal which coordinate mode is active.
    // minecraft = true  → save/load in block coordinates (requires JSON metadata)
    // minecraft = false → save/load in normalised image coordinates (0.0–1.0)
    void setMinecraftMode(bool enabled) { minecraftMode_ = enabled; }
    bool isMinecraftMode()        const { return minecraftMode_; }

    double getWorldWidth()  const { return worldWidth; }
    double getWorldHeight() const { return worldHeight; }

    Vec2 blockToWorld(const BlockCoord& block) const;
    BlockCoord worldToBlock(const Vec2& worldPos) const;

    RegionTree& getRegionTree()             { return regionTree; }
    const RegionTree& getRegionTree() const { return regionTree; }

    SelectionState& getSelection()             { return selection; }
    const SelectionState& getSelection() const { return selection; }

    EditState& getEditState()             { return editState; }
    const EditState& getEditState() const { return editState; }

    void deleteEditPoint();

    void setPendingParent(RegionId id) { pendingParentId = id; hasPendingParent = true; }
    void clearPendingParent()          { hasPendingParent = false; }
    bool isPendingParentSet() const    { return hasPendingParent; }

private:
    Camera         camera;
    Input          input;
    RegionTree     regionTree;
    SelectionState selection;
    EditState      editState;

    bool     hasPendingParent = false;
    RegionId pendingParentId  = 0;

    double worldWidth  = 0.0;
    double worldHeight = 0.0;

    bool minecraftMode_ = false;

    int worldMinBlockX = 0;
    int worldMinBlockZ = 0;
    int worldMaxBlockX = 0;
    int worldMaxBlockZ = 0;

    void updateEditMode(GLFWwindow* window);

    void regionBoundingBox(const Region& region,
                           double& minX, double& minY,
                           double& maxX, double& maxY) const;

    bool childrenBoundingBox(const Region& region,
                             double& minX, double& minY,
                             double& maxX, double& maxY) const;
};
