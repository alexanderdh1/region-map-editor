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
    RectCorner,  // index 0-3: TL, TR, BR, BL
    PolyPoint,   // index = point index in geometry.points
};

struct EditState
{
    Region*        target      = nullptr;
    EditHandleType handleType  = EditHandleType::None;
    int            handleIndex = -1;

    // World-space position of the dragged point/corner at drag start.
    // Used with total mouse delta for constraint-safe absolute positioning.
    Vec2 dragOriginWorld { 0.0, 0.0 };

    bool isActive()   const { return target != nullptr; }
    bool isDragging() const { return handleType != EditHandleType::None; }

    void clear()
    {
        target         = nullptr;
        handleType     = EditHandleType::None;
        handleIndex    = -1;
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

    EditState& getEditState()             { return editState; }
    const EditState& getEditState() const { return editState; }

    // Deletes the selected polygon point, connecting its two neighbours.
    // No-op if fewer than 4 points or no PolyPoint handle is selected.
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
