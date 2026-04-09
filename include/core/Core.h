#pragma once

#include "rendering/Camera.h"
#include "input/Input.h"
#include "math/MapCoord.h"
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
    int            hoveredHandleIndex = -1;   // index under cursor this frame (-1 = none)
    Vec2 dragOriginWorld { 0.0, 0.0 };

    bool isActive()   const { return target != nullptr; }
    bool isDragging() const { return handleType != EditHandleType::None; }

    void clear()
    {
        target          = nullptr;
        handleType      = EditHandleType::None;
        handleIndex     = -1;
        hoveredHandleIndex = -1;
        dragOriginWorld = { 0.0, 0.0 };
    }

    void clearDrag()
    {
        handleType      = EditHandleType::None;
        handleIndex     = -1;
        hoveredHandleIndex = -1;
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

    void setMapSize(double width, double height);

    void setMapCoordBounds(
        int minX, int minY,
        int maxX, int maxY
    );

    // Called by MapLoader to signal which coordinate mode is active.
    // block = true  → save/load in block coordinates (requires JSON metadata)
    // block = false → save/load in normalised image coordinates (0.0–1.0)
    void setCoordMode(bool enabled) { coordMode_ = enabled; }
    bool isCoordMode()        const { return coordMode_; }

    double getMapWidth()  const { return mapWidth; }
    double getMapHeight() const { return mapHeight; }

    Vec2 coordToWorld(const MapCoord& block) const;
    MapCoord worldToCoord(const Vec2& worldPos) const;

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

    double mapWidth  = 0.0;
    double mapHeight = 0.0;

    bool coordMode_ = false;

    int worldMinX = 0;
    int worldMinY = 0;
    int worldMaxX = 0;
    int worldMaxY = 0;

    void updateEditMode(GLFWwindow* window);

    void regionBoundingBox(const Region& region,
                           double& minX, double& minY,
                           double& maxX, double& maxY) const;

    bool childrenBoundingBox(const Region& region,
                             double& minX, double& minY,
                             double& maxX, double& maxY) const;
};
