#include "core/Core.h"
#include "input/Input.h"
#include "math/Vec2.h"
#include "data/Region.h"
#include "data/RegionSerializer.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <memory>
#include <cmath>

static constexpr double MIN_REGION_SIZE  = 4.0;
static constexpr double HANDLE_RADIUS_PX = 12.0;
static constexpr double EDGE_HIT_PX     = 8.0;

Core::Core() = default;

// ---------------------------------------------------------------
// Bounding box helpers
// ---------------------------------------------------------------

void Core::regionBoundingBox(const Region& region,
                             double& minX, double& minY,
                             double& maxX, double& maxY) const
{
    auto pts = region.geometry.getPoints();
    minX = maxX = pts[0].x;
    minY = maxY = pts[0].y;
    for (const auto& p : pts)
    {
        minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
    }
}

bool Core::childrenBoundingBox(const Region& region,
                               double& minX, double& minY,
                               double& maxX, double& maxY) const
{
    if (region.children.empty()) return false;
    bool first = true;
    for (const auto& child : region.children)
    {
        double cx0, cy0, cx1, cy1;
        regionBoundingBox(*child, cx0, cy0, cx1, cy1);
        if (first) { minX = cx0; minY = cy0; maxX = cx1; maxY = cy1; first = false; }
        else
        {
            minX = std::min(minX, cx0); maxX = std::max(maxX, cx1);
            minY = std::min(minY, cy0); maxY = std::max(maxY, cy1);
        }
    }
    return true;
}

// ---------------------------------------------------------------
// Main update
// ---------------------------------------------------------------

void Core::update(GLFWwindow* window)
{
    Camera& cam = getCamera();

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    Vec2 mouseScreen{ mouseX, mouseY };

    // ZOOM always works
    if (input.hasZoomDelta())
    {
        double zoomSteps = input.consumeZoomDelta();
        const double zoomFactor = 1.1;
        Vec2 worldBefore = cam.screenToWorld(mouseScreen);
        if (zoomSteps > 0) cam.zoomBy(zoomFactor);
        else               cam.zoomBy(1.0 / zoomFactor);
        Vec2 worldAfter = cam.screenToWorld(mouseScreen);
        cam.position.x += (worldBefore.x - worldAfter.x);
        cam.position.y += (worldBefore.y - worldAfter.y);
    }

    cam.clampToBounds(worldWidth, worldHeight);

    if (input.getDrawTool() == DrawTool::Edit)
    {
        updateEditMode(window);
        return;
    }

    // PAN
    if (input.hasPanDelta())
    {
        Vec2 delta = input.consumePanDelta();
        cam.panBy(delta);
    }

    auto addRegion = [&](std::unique_ptr<Region> region)
    {
        if (hasPendingParent)
        {
            regionTree.addChildRegion(pendingParentId, std::move(region));
            hasPendingParent = false;
        }
        else
        {
            regionTree.addRegion(std::move(region));
        }
        RegionSerializer::save(regionTree, "regions.json", *this);
    };

    auto isInsideParent = [&](const Vec2& worldPt) -> bool
    {
        if (!hasPendingParent) return true;
        Region* parent = regionTree.findById(pendingParentId);
        if (!parent) return true;
        return parent->geometry.contains(worldPt);
    };

    // Auto sub-region: if a region is selected and drawing starts inside it,
    // automatically treat the new shape as a child — no right-click needed.
    auto autoSetPendingParent = [&](const Vec2& worldPt)
    {
        if (hasPendingParent) return; // already set explicitly
        Region* selected = selection.selectedRegion;
        if (!selected) return;
        if (selected->geometry.contains(worldPt))
        {
            pendingParentId  = selected->id;
            hasPendingParent = true;
        }
    };

    // RECT
    if (input.isRectJustStarted())
    {
        Vec2 worldStart = cam.screenToWorld(input.getDrawStart());
        autoSetPendingParent(worldStart);
        if (!isInsideParent(worldStart))
            input.cancelRect();
        else
        {
            input.setDrawStartWorld(worldStart);
            input.consumeRectJustStarted();
        }
    }

    if (input.hasCompletedRect())
    {
        Vec2 worldA = input.getDrawStartWorld();
        Vec2 cur = input.getDrawCurrent();
        cur.x = std::max(0.0, std::min(cur.x, cam.viewportSize.x));
        cur.y = std::max(0.0, std::min(cur.y, cam.viewportSize.y));
        Vec2 worldB = cam.screenToWorld(cur);

        if (hasPendingParent)
        {
            Region* parent = regionTree.findById(pendingParentId);
            if (parent)
            {
                const auto& pts = parent->geometry.getPoints();
                double minX = pts[0].x, maxX = pts[0].x;
                double minY = pts[0].y, maxY = pts[0].y;
                for (const auto& p : pts)
                {
                    minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
                    minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
                }
                worldB.x = std::clamp(worldB.x, minX, maxX);
                worldB.y = std::clamp(worldB.y, minY, maxY);
            }
        }

        RegionGeometry geom;
        geom.type    = GeometryType::Rectangle;
        geom.rectMin = { std::min(worldA.x, worldB.x), std::min(worldA.y, worldB.y) };
        geom.rectMax = { std::max(worldA.x, worldB.x), std::max(worldA.y, worldB.y) };

        bool tooSmall =
            (geom.rectMax.x - geom.rectMin.x) < MIN_REGION_SIZE ||
            (geom.rectMax.y - geom.rectMin.y) < MIN_REGION_SIZE;

        if (geom.isValid() && !tooSmall)
        {
            auto region      = std::make_unique<Region>();
            region->id       = regionTree.nextId();
            region->geometry = geom;
            addRegion(std::move(region));
        }
        input.consumeCompletedRect();
    }

    // POLYGON
    if (input.hasPendingPolyPoint())
    {
        Vec2 screenPt = input.consumePendingPolyPoint();
        Vec2 worldPt  = cam.screenToWorld(screenPt);

        // Auto sub-region on first polygon point
        if (input.getPolygonWorldPoints().empty())
            autoSetPendingParent(worldPt);

        if (isInsideParent(worldPt))
        {
            const std::vector<Vec2>& existing = input.getPolygonWorldPoints();
            bool closedByFirstPoint = false;

            if (existing.size() >= 3)
            {
                Vec2 firstScreen = cam.worldToScreen(existing[0]);
                double dist = std::hypot(
                    screenPt.x - firstScreen.x,
                    screenPt.y - firstScreen.y
                );
                if (dist <= 12.0)
                {
                    input.closePolygon();
                    closedByFirstPoint = true;
                }
            }
            if (!closedByFirstPoint)
                input.addPolygonWorldPoint(worldPt);
        }
    }

    if (input.hasCompletedPolygon())
    {
        std::vector<Vec2> worldPoints = input.consumeCompletedPolygon();
        RegionGeometry geom;
        geom.type   = GeometryType::Polygon;
        geom.points = worldPoints;

        if (geom.isValid())
        {
            const auto& pts = geom.points;
            double minX = pts[0].x, maxX = pts[0].x;
            double minY = pts[0].y, maxY = pts[0].y;
            for (const auto& p : pts)
            {
                minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
                minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
            }
            bool tooSmall = (maxX - minX) < MIN_REGION_SIZE &&
                            (maxY - minY) < MIN_REGION_SIZE;
            if (!tooSmall)
            {
                auto region      = std::make_unique<Region>();
                region->id       = regionTree.nextId();
                region->geometry = geom;
                addRegion(std::move(region));
            }
        }
    }

    // CLICK
    if (input.hasClick())
    {
        Vec2 screenPos = input.consumeClick();
        Vec2 worldPos  = cam.screenToWorld(screenPos);

        Region* hit = nullptr;
        regionTree.forEach([&](Region& r)
        {
            if (!r.hidden && r.geometry.contains(worldPos)) hit = &r;
        });

        if (hit)
        {
            selection.viewStack.clear();
            Region* ancestor = hit->parent;
            while (ancestor)
            {
                selection.viewStack.insert(selection.viewStack.begin(), ancestor);
                ancestor = ancestor->parent;
            }
            selection.selectedRegion = hit;
            selection.popupOpen      = true;
        }
        else
        {
            selection.clear();
        }
    }

    // Right-click is not used in the current UI — input is consumed to keep
    // the Input state clean, but no action is taken.
    if (input.hasRightClick())
        input.consumeRightClick();
}

// ---------------------------------------------------------------
// Edit mode update
// ---------------------------------------------------------------

void Core::updateEditMode(GLFWwindow* window)
{
    Camera& cam = getCamera();

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    Vec2 mouseScreen{ mouseX, mouseY };

    // Pan is disabled in edit mode.

    // ---- Update hover highlight every frame ----
    // This drives the visual highlight in RegionRenderer independently of drag state.
    editState.hoveredHandleIndex = -1;
    if (editState.isActive() && !editState.isDragging())
    {
        const Region& hoverRegion = *editState.target;
        if (hoverRegion.geometry.type == GeometryType::Polygon)
        {
            const auto& pts = hoverRegion.geometry.points;
            for (int i = 0; i < static_cast<int>(pts.size()); i++)
            {
                Vec2 s = cam.worldToScreen(pts[i]);
                if (std::hypot(mouseScreen.x - s.x, mouseScreen.y - s.y) <= HANDLE_RADIUS_PX)
                {
                    editState.hoveredHandleIndex = i;
                    break;
                }
            }
        }
        else if (hoverRegion.geometry.type == GeometryType::Rectangle)
        {
            Vec2 worldCorners[4] = {
                { hoverRegion.geometry.rectMin.x, hoverRegion.geometry.rectMax.y },
                { hoverRegion.geometry.rectMax.x, hoverRegion.geometry.rectMax.y },
                { hoverRegion.geometry.rectMax.x, hoverRegion.geometry.rectMin.y },
                { hoverRegion.geometry.rectMin.x, hoverRegion.geometry.rectMin.y },
            };
            for (int i = 0; i < 4; i++)
            {
                Vec2 s = cam.worldToScreen(worldCorners[i]);
                if (std::hypot(mouseScreen.x - s.x, mouseScreen.y - s.y) <= HANDLE_RADIUS_PX)
                {
                    editState.hoveredHandleIndex = i;
                    break;
                }
            }
        }
    }

    // ---- Polygon edge insertion (plain click on edge) ----
    if (input.hasEditClick())
    {
        Vec2 clickScreen = input.consumeEditClick();
        Vec2 clickWorld  = cam.screenToWorld(clickScreen);

        if (editState.isActive() &&
            editState.target->geometry.type == GeometryType::Polygon)
        {
            Region& region = *editState.target;
            auto& pts = region.geometry.points;
            int n = static_cast<int>(pts.size());

            double bestDist = EDGE_HIT_PX;
            int    bestEdge = -1;

            for (int i = 0; i < n; i++)
            {
                int j = (i + 1) % n;
                Vec2 sA = cam.worldToScreen(pts[i]);
                Vec2 sB = cam.worldToScreen(pts[j]);

                double dA = std::hypot(clickScreen.x - sA.x, clickScreen.y - sA.y);
                double dB = std::hypot(clickScreen.x - sB.x, clickScreen.y - sB.y);
                if (dA < HANDLE_RADIUS_PX || dB < HANDLE_RADIUS_PX)
                {
                    bestEdge = -1;
                    break;
                }

                Vec2 ab { sB.x - sA.x, sB.y - sA.y };
                Vec2 ap { clickScreen.x - sA.x, clickScreen.y - sA.y };
                double lenSq = ab.x * ab.x + ab.y * ab.y;
                double t = (lenSq > 0.0)
                    ? std::clamp((ap.x * ab.x + ap.y * ab.y) / lenSq, 0.0, 1.0)
                    : 0.0;
                Vec2 closest { sA.x + t * ab.x, sA.y + t * ab.y };
                double dist = std::hypot(
                    clickScreen.x - closest.x,
                    clickScreen.y - closest.y
                );
                if (dist < bestDist) { bestDist = dist; bestEdge = i; }
            }

            if (bestEdge >= 0)
            {
                pts.insert(pts.begin() + bestEdge + 1, clickWorld);
                RegionSerializer::save(regionTree, "regions.json", *this);
            }
        }
        // Note: target selection is no longer done here.
        // The edit target is always the selected region (set when entering edit mode).
        return;
    }

    // ---- Drag start: identify handle ----
    if (input.hasEditDragStart())
    {
        Vec2 startScreen = input.consumeEditDragStart();
        editState.clearDrag();

        if (!editState.isActive()) return;

        Region& region = *editState.target;

        if (region.geometry.type == GeometryType::Rectangle)
        {
            // Corner world positions: TL, TR, BR, BL
            Vec2 worldCorners[4] = {
                { region.geometry.rectMin.x, region.geometry.rectMax.y },
                { region.geometry.rectMax.x, region.geometry.rectMax.y },
                { region.geometry.rectMax.x, region.geometry.rectMin.y },
                { region.geometry.rectMin.x, region.geometry.rectMin.y },
            };
            for (int i = 0; i < 4; i++)
            {
                Vec2 s = cam.worldToScreen(worldCorners[i]);
                double dist = std::hypot(startScreen.x - s.x, startScreen.y - s.y);
                if (dist <= HANDLE_RADIUS_PX)
                {
                    editState.handleType     = EditHandleType::RectCorner;
                    editState.handleIndex    = i;
                    // Store the world position of this corner at drag start
                    editState.dragOriginWorld = worldCorners[i];
                    return;
                }
            }
        }
        else if (region.geometry.type == GeometryType::Polygon)
        {
            const auto& pts = region.geometry.points;
            for (int i = 0; i < static_cast<int>(pts.size()); i++)
            {
                Vec2 s = cam.worldToScreen(pts[i]);
                double dist = std::hypot(startScreen.x - s.x, startScreen.y - s.y);
                if (dist <= HANDLE_RADIUS_PX)
                {
                    editState.handleType      = EditHandleType::PolyPoint;
                    editState.handleIndex     = i;
                    // Store the world position of this point at drag start
                    editState.dragOriginWorld = pts[i];
                    return;
                }
            }
        }
    }

    // ---- Dragging ----
    // Use ABSOLUTE positioning: new position = dragOriginWorld + totalMouseDelta/zoom.
    // This means constraints never cause offset accumulation — if the mouse moves
    // 1000px past a constraint boundary and reverses, the geometry only starts
    // moving again exactly when the mouse returns to the constraint point.
    if (input.isEditDragging())
    {
        // Per-frame delta must always be consumed to prevent stale accumulation
        input.getEditDragDelta();

        if (!editState.isDragging()) return;

        // Convert total mouse movement to world space
        Vec2 totalScreen = input.getEditDragTotalDelta();
        double worldDx =  totalScreen.x / cam.zoom;
        double worldDy = -totalScreen.y / cam.zoom;

        // Desired absolute world position of the dragged point
        Vec2 desiredWorld {
            editState.dragOriginWorld.x + worldDx,
            editState.dragOriginWorld.y + worldDy
        };

        Region& region = *editState.target;

        if (editState.handleType == EditHandleType::RectCorner)
        {
            Vec2 newMin = region.geometry.rectMin;
            Vec2 newMax = region.geometry.rectMax;

            switch (editState.handleIndex)
            {
                case 0: newMin.x = desiredWorld.x; newMax.y = desiredWorld.y; break; // TL
                case 1: newMax.x = desiredWorld.x; newMax.y = desiredWorld.y; break; // TR
                case 2: newMax.x = desiredWorld.x; newMin.y = desiredWorld.y; break; // BR
                case 3: newMin.x = desiredWorld.x; newMin.y = desiredWorld.y; break; // BL
            }

            // Enforce minimum size — keep opposite corner fixed
            if (newMax.x - newMin.x < MIN_REGION_SIZE)
            {
                if (editState.handleIndex == 0 || editState.handleIndex == 3)
                    newMin.x = newMax.x - MIN_REGION_SIZE;
                else
                    newMax.x = newMin.x + MIN_REGION_SIZE;
            }
            if (newMax.y - newMin.y < MIN_REGION_SIZE)
            {
                if (editState.handleIndex == 2 || editState.handleIndex == 3)
                    newMin.y = newMax.y - MIN_REGION_SIZE;
                else
                    newMax.y = newMin.y + MIN_REGION_SIZE;
            }

            // Build the four corners of the proposed new rect
            Vec2 proposed[4] = {
                { newMin.x, newMax.y }, // TL
                { newMax.x, newMax.y }, // TR
                { newMax.x, newMin.y }, // BR
                { newMin.x, newMin.y }, // BL
            };

            // Parent containment: every corner of this rect must be inside parent.
            // We use contains() so non-rectangular parents work correctly.
            if (region.parent)
            {
                for (const Vec2& corner : proposed)
                {
                    if (!region.parent->geometry.contains(corner))
                    {
                        // Desired position violates parent — keep current geometry
                        return;
                    }
                }
            }

            // Children containment: rect cannot shrink below children bounding box
            double cMinX, cMinY, cMaxX, cMaxY;
            if (childrenBoundingBox(region, cMinX, cMinY, cMaxX, cMaxY))
            {
                if (newMin.x > cMinX || newMax.x < cMaxX ||
                    newMin.y > cMinY || newMax.y < cMaxY)
                    return;
            }

            region.geometry.rectMin = newMin;
            region.geometry.rectMax = newMax;
        }
        else if (editState.handleType == EditHandleType::PolyPoint)
        {
            int idx = editState.handleIndex;
            auto& pts = region.geometry.points;
            if (idx < 0 || idx >= static_cast<int>(pts.size())) return;

            Vec2 newPt = desiredWorld;

            // Parent containment: the moved point must be inside parent
            if (region.parent && !region.parent->geometry.contains(newPt))
                return; // hold position — mouse will re-engage when it returns

            // Children containment: polygon bounding box must still cover children
            double cMinX, cMinY, cMaxX, cMaxY;
            if (childrenBoundingBox(region, cMinX, cMinY, cMaxX, cMaxY))
            {
                double newRegMinX = newPt.x, newRegMaxX = newPt.x;
                double newRegMinY = newPt.y, newRegMaxY = newPt.y;
                for (int i = 0; i < static_cast<int>(pts.size()); i++)
                {
                    if (i == idx) continue;
                    newRegMinX = std::min(newRegMinX, pts[i].x);
                    newRegMaxX = std::max(newRegMaxX, pts[i].x);
                    newRegMinY = std::min(newRegMinY, pts[i].y);
                    newRegMaxY = std::max(newRegMaxY, pts[i].y);
                }
                if (newRegMinX > cMinX || newRegMaxX < cMaxX ||
                    newRegMinY > cMinY || newRegMaxY < cMaxY)
                    return;
            }

            pts[idx] = newPt;
        }
    }

    // ---- Drag end: auto-save ----
    if (input.hasEditDragEnd())
    {
        input.consumeEditDragEnd();
        RegionSerializer::save(regionTree, "regions.json", *this);
    }
}

// ---------------------------------------------------------------
// Delete polygon point
// ---------------------------------------------------------------

void Core::deleteEditPoint()
{
    if (!editState.isActive()) return;
    if (editState.handleType != EditHandleType::PolyPoint) return;

    Region& region = *editState.target;
    auto& pts = region.geometry.points;
    int n   = static_cast<int>(pts.size());
    int idx = editState.handleIndex;

    if (idx < 0 || idx >= n) return;
    if (n <= 3) return;

    pts.erase(pts.begin() + idx);
    // Clear the entire drag state so the dragging block that runs later
    // in the same frame cannot write through a now-stale handleIndex.
    editState.clearDrag();

    RegionSerializer::save(regionTree, "regions.json", *this);
}

// ---------------------------------------------------------------
// Camera / world helpers
// ---------------------------------------------------------------

Camera& Core::getCamera() { return camera; }
const Camera& Core::getCamera() const { return camera; }
Input& Core::getInput() { return input; }
const Input& Core::getInput() const { return input; }

void Core::setWorldSize(double width, double height)
{
    worldWidth  = width;
    worldHeight = height;
    double minZoomX = camera.viewportSize.x / worldWidth;
    double minZoomY = camera.viewportSize.y / worldHeight;
    camera.minZoom = std::max(minZoomX, minZoomY);
}

void Core::setWorldBlockBounds(
    int minBlockX, int minBlockZ,
    int maxBlockX, int maxBlockZ)
{
    worldMinBlockX = minBlockX;
    worldMinBlockZ = minBlockZ;
    worldMaxBlockX = maxBlockX;
    worldMaxBlockZ = maxBlockZ;
}

Vec2 Core::blockToWorld(const BlockCoord& block) const
{
    double worldX = block.x - worldMinBlockX;
    double worldZ = worldMaxBlockZ - block.z;
    worldX -= worldWidth  / 2.0;
    worldZ -= worldHeight / 2.0;
    return { worldX, worldZ };
}

BlockCoord Core::worldToBlock(const Vec2& worldPos) const
{
    double localX = worldPos.x + worldWidth  / 2.0;
    double localZ = worldPos.y + worldHeight / 2.0;
    int blockX = static_cast<int>(localX) + worldMinBlockX;
    int blockZ = worldMaxBlockZ - static_cast<int>(localZ);
    return { blockX, blockZ };
}
