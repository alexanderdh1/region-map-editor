#pragma once
#include "math/Vec2.h"
#include <vector>

enum class InputMode
{
    Navigate,
    DrawRect,
    DrawPolygon
};

enum class DrawTool
{
    Rectangle,
    Polygon
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

    // Screen-space cursor position for live preview line
    Vec2 getPolygonCursor() const { return drawCurrent; }

    // World-space points already confirmed by Core
    const std::vector<Vec2>& getPolygonWorldPoints() const { return polyWorldPoints; }
    void addPolygonWorldPoint(const Vec2& worldPt) { polyWorldPoints.push_back(worldPt); }

    // One frame flag: a new screen-space point is ready for Core to convert
    bool hasPendingPolyPoint() const  { return pendingPolyPoint; }
    Vec2 consumePendingPolyPoint()
    {
        pendingPolyPoint = false;
        return pendingPolyPos;
    }

    // One frame flag: polygon is complete, world points are in polyWorldPoints
    bool hasCompletedPolygon() const;
    std::vector<Vec2> consumeCompletedPolygon();

    void cancelPolygon();
    void closePolygon(); // finalise with current world points (no new point added)

    // Click (left, no shift, no drag)
    bool hasClick() const;
    Vec2 consumeClick();

    // Right-click
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

    // Rectangle
    bool drawing         = false;
    bool rectCompleted   = false;
    bool rectJustStarted = false;  // true for exactly one frame on press
    Vec2 drawStart      { 0.0, 0.0 };
    Vec2 drawStartWorld { 0.0, 0.0 };
    Vec2 drawCurrent    { 0.0, 0.0 };

    // Polygon
    bool              polyDrawing      = false;
    bool              polyCompleted    = false;
    bool              pendingPolyPoint = false;
    Vec2              pendingPolyPos   { 0.0, 0.0 };
    std::vector<Vec2> polyWorldPoints;  // world-space, filled by Core
    std::vector<Vec2> completedPoly;

    // Double-click detection
    double lastClickTime = 0.0;
    Vec2   lastClickPos  { 0.0, 0.0 };

    // Click
    bool clickPending      = false;
    Vec2 clickPos          { 0.0, 0.0 };

    // Right-click
    bool rightClickPending = false;
    Vec2 rightClickPos     { 0.0, 0.0 };
};
