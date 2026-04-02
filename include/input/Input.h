#pragma once
#include "math/Vec2.h"
#include <vector>

enum class InputMode
{
    Navigate,
    DrawRect,
    DrawPolygon,
    Edit
};

enum class DrawTool
{
    Navigate,
    Rectangle,
    Polygon,
    Edit
};

class Input {
public:
    void onMouseButton(bool pressed, const Vec2& mousePos, bool shiftHeld);
    void onMouseButtonRight(bool pressed, const Vec2& mousePos);
    void onMouseMove(const Vec2& mousePos);
    void onScroll(double yOffset);

    void setDrawTool(DrawTool tool) { activeTool = tool; }
    DrawTool getDrawTool() const    { return activeTool; }

    // --- Navigation ---
    bool hasPanDelta() const;
    Vec2 consumePanDelta();

    bool hasZoomDelta() const;
    double consumeZoomDelta();

    // --- Rectangle drawing ---
    InputMode getMode() const { return mode; }

    bool isDrawingRect() const;
    Vec2 getDrawStart()      const { return drawStart; }
    Vec2 getDrawCurrent()    const { return drawCurrent; }
    Vec2 getDrawStartWorld() const { return drawStartWorld; }
    void setDrawStartWorld(const Vec2& w) { drawStartWorld = w; }

    bool isRectJustStarted() const { return rectJustStarted; }
    void consumeRectJustStarted()  { rectJustStarted = false; }

    bool hasCompletedRect() const;
    void consumeCompletedRect();
    void cancelRect();

    // --- Polygon drawing ---
    bool isDrawingPolygon() const { return polyDrawing; }

    Vec2 getPolygonCursor() const { return drawCurrent; }

    const std::vector<Vec2>& getPolygonWorldPoints() const { return polyWorldPoints; }
    void addPolygonWorldPoint(const Vec2& worldPt) { polyWorldPoints.push_back(worldPt); }

    bool hasPendingPolyPoint() const  { return pendingPolyPoint; }
    Vec2 consumePendingPolyPoint()
    {
        pendingPolyPoint = false;
        return pendingPolyPos;
    }

    bool hasCompletedPolygon() const;
    std::vector<Vec2> consumeCompletedPolygon();

    void cancelPolygon();
    void closePolygon();

    // --- Edit mode ---
    // A drag on a handle or inside a region was just started (press, no shift)
    bool hasEditDragStart() const     { return editDragStartPending; }
    Vec2 consumeEditDragStart()       { editDragStartPending = false; return editDragStartPos; }

    // Mouse moved while edit-dragging
    bool isEditDragging() const       { return editDragging; }
    Vec2 getEditDragCurrent() const   { return drawCurrent; }
    Vec2 getEditDragDelta()           // delta since last frame, resets each call
    {
        Vec2 d = editDragDelta;
        editDragDelta = { 0.0, 0.0 };
        return d;
    }

    // Drag ended
    bool hasEditDragEnd() const       { return editDragEndPending; }
    void consumeEditDragEnd()         { editDragEndPending = false; }

    // Plain click in edit mode (no drag) — used for edge insertion and target selection
    bool hasEditClick() const         { return editClickPending; }
    Vec2 consumeEditClick()           { editClickPending = false; return editClickPos; }

    void cancelEdit();

    // --- Click (left, no shift, no drag, non-edit mode) ---
    bool hasClick() const;
    Vec2 consumeClick();

    // --- Right-click ---
    bool hasRightClick() const  { return rightClickPending; }
    Vec2 consumeRightClick()    { rightClickPending = false; return rightClickPos; }

private:
    DrawTool  activeTool = DrawTool::Rectangle;
    InputMode mode       = InputMode::Navigate;

    // Navigate
    bool   dragging     = false;
    bool   didDrag      = false;
    Vec2   lastMousePos { 0.0, 0.0 };
    Vec2   panDelta     { 0.0, 0.0 };
    double zoomDelta    = 0.0;

    // Rectangle drawing
    bool drawing         = false;
    bool rectCompleted   = false;
    bool rectJustStarted = false;
    Vec2 drawStart      { 0.0, 0.0 };
    Vec2 drawStartWorld { 0.0, 0.0 };
    Vec2 drawCurrent    { 0.0, 0.0 };

    // Polygon drawing
    bool              polyDrawing      = false;
    bool              polyCompleted    = false;
    bool              pendingPolyPoint = false;
    Vec2              pendingPolyPos   { 0.0, 0.0 };
    std::vector<Vec2> polyWorldPoints;
    std::vector<Vec2> completedPoly;

    // Double-click detection
    double lastClickTime = 0.0;
    Vec2   lastClickPos  { 0.0, 0.0 };

    // Click
    bool clickPending = false;
    Vec2 clickPos     { 0.0, 0.0 };

    // Right-click
    bool rightClickPending = false;
    Vec2 rightClickPos     { 0.0, 0.0 };

    // Edit mode
    bool editDragging         = false;
    bool editDragStartPending = false;
    bool editDragEndPending   = false;
    bool editDidDrag          = false;
    bool editClickPending     = false;
    Vec2 editDragStartPos  { 0.0, 0.0 };
    Vec2 editDragDelta     { 0.0, 0.0 };
    Vec2 editLastMousePos  { 0.0, 0.0 };
    Vec2 editClickPos      { 0.0, 0.0 };
};
