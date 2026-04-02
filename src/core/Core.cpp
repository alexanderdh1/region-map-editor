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
static constexpr double HANDLE_RADIUS_PX = 8.0;
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
    Camera& camera = getCamera();

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    Vec2 mouseScreen{ mouseX, mouseY };

    // PAN
    if (input.hasPanDelta()) {
        Vec2 delta = input.consumePanDelta();
        camera.panBy(delta);
    }

    // ZOOM
    if (input.hasZoomDelta()) {
        double zoomSteps = input.consumeZoomDelta();
        const double zoomFactor = 1.1;
        Vec2 worldBefore = camera.screenToWorld(mouseScreen);
        if (zoomSteps > 0) camera.zoomBy(zoomFactor);
        else               camera.zoomBy(1.0 / zoomFactor);
        Vec2 worldAfter = camera.screenToWorld(mouseScreen);
        camera.position.x += (worldBefore.x - worldAfter.x);
        camera.position.y += (worldBefore.y - worldAfter.y);
    }

    camera.clampToBounds(worldWidth, worldHeight);

    // Edit mode has its own update path
    if (input.getDrawTool() == DrawTool::Edit)
    {
        updateEditMode(window);
        return;
    }

    // Helper: add region and auto-save
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
        RegionSerializer::save(regionTree, "regions.json");
    };

    auto isInsideParent = [&](const Vec2& worldPt) -> bool
    {
        if (!hasPendingParent) return true;
        Region* parent = regionTree.findById(pendingParentId);
        if (!parent) return true;
        return parent->geometry.contains(worldPt);
    };

    // RECT — capture start
    if (input.isRectJustStarted())
    {
        Vec2 worldStart = camera.screenToWorld(input.getDrawStart());
        if (!isInsideParent(worldStart))
            input.cancelRect();
        else
        {
            input.setDrawStartWorld(worldStart);
            input.consumeRectJustStarted();
        }
    }

    // RECT — finalise
    if (input.hasCompletedRect())
    {
        Vec2 worldA = input.getDrawStartWorld();
        Vec2 cur = input.getDrawCurrent();
        cur.x = std::max(0.0, std::min(cur.x, camera.viewportSize.x));
        cur.y = std::max(0.0, std::min(cur.y, camera.viewportSize.y));
        Vec2 worldB = camera.screenToWorld(cur);

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

    // POLYGON — add point
    if (input.hasPendingPolyPoint())
    {
        Vec2 screenPt = input.consumePendingPolyPoint();
        Vec2 worldPt  = camera.screenToWorld(screenPt);

        if (isInsideParent(worldPt))
        {
            const std::vector<Vec2>& existing = input.getPolygonWorldPoints();
            bool closedByFirstPoint = false;

            if (existing.size() >= 3)
            {
                Vec2 firstScreen = camera.worldToScreen(existing[0]);
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

    // POLYGON — finalise
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

    // CLICK — select region
    if (input.hasClick())
    {
        Vec2 screenPos = input.consumeClick();
        Vec2 worldPos  = camera.screenToWorld(screenPos);

        Region* hit = nullptr;
        regionTree.forEach([&](Region& r)
        {
            if (r.geometry.contains(worldPos)) hit = &r;
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

    // RIGHT-CLICK
    if (input.hasRightClick())
    {
        Vec2 screenPos = input.consumeRightClick();
        Vec2 worldPos  = camera.screenToWorld(screenPos);

        Region* hit = nullptr;
        regionTree.forEach([&](Region& r)
        {
            if (r.geometry.contains(worldPos)) hit = &r;
        });

        selection.contextRegion     = hit;
        selection.contextMenuOpen   = (hit != nullptr);
        selection.contextMenuScreen = screenPos;
    }
}

// ---------------------------------------------------------------
// Edit mode update
// ---------------------------------------------------------------

void Core::updateEditMode(GLFWwindow* /*window*/)
{
    const Camera& camera = getCamera();
    Vec2 mouseScreen = input.getEditDragCurrent();

    // ---- Plain click: select target or insert edge point ----
    if (input.hasEditClick())
    {
        Vec2 clickScreen = input.consumeEditClick();
        Vec2 clickWorld  = camera.screenToWorld(clickScreen);

        // Polygon edge insertion on current target
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
                Vec2 sA = camera.worldToScreen(pts[i]);
                Vec2 sB = camera.worldToScreen(pts[j]);

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
                RegionSerializer::save(regionTree, "regions.json");
                return;
            }
        }

        // Hit-test to set new edit target
        Region* hit = nullptr;
        regionTree.forEach([&](Region& r)
        {
            if (r.geometry.contains(clickWorld)) hit = &r;
        });

        editState.target = hit;
        editState.clearDrag();
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
            Vec2 corners[4] = {
                camera.worldToScreen({ region.geometry.rectMin.x, region.geometry.rectMax.y }),
                camera.worldToScreen({ region.geometry.rectMax.x, region.geometry.rectMax.y }),
                camera.worldToScreen({ region.geometry.rectMax.x, region.geometry.rectMin.y }),
                camera.worldToScreen({ region.geometry.rectMin.x, region.geometry.rectMin.y }),
            };
            for (int i = 0; i < 4; i++)
            {
                double dist = std::hypot(
                    startScreen.x - corners[i].x,
                    startScreen.y - corners[i].y
                );
                if (dist <= HANDLE_RADIUS_PX)
                {
                    editState.handleType  = EditHandleType::RectCorner;
                    editState.handleIndex = i;
                    return;
                }
            }
        }
        else if (region.geometry.type == GeometryType::Polygon)
        {
            const auto& pts = region.geometry.points;
            for (int i = 0; i < static_cast<int>(pts.size()); i++)
            {
                Vec2 s = camera.worldToScreen(pts[i]);
                double dist = std::hypot(
                    startScreen.x - s.x,
                    startScreen.y - s.y
                );
                if (dist <= HANDLE_RADIUS_PX)
                {
                    editState.handleType  = EditHandleType::PolyPoint;
                    editState.handleIndex = i;
                    return;
                }
            }
        }
    }

    // ---- Dragging: apply delta ----
    if (input.isEditDragging())
    {
        // Always consume the delta — even if we can't apply it.
        // Bug fix: not consuming caused delta accumulation, making the
        // handle drift away from the cursor when blocked by a constraint.
        Vec2 screenDelta = input.getEditDragDelta();

        if (!editState.isDragging()) return;
        if (screenDelta.x == 0.0 && screenDelta.y == 0.0) return;

        double worldDx =  screenDelta.x / camera.zoom;
        double worldDy = -screenDelta.y / camera.zoom;

        Region& region = *editState.target;

        if (editState.handleType == EditHandleType::RectCorner)
        {
            Vec2 newMin = region.geometry.rectMin;
            Vec2 newMax = region.geometry.rectMax;

            switch (editState.handleIndex)
            {
                case 0: newMin.x += worldDx; newMax.y += worldDy; break; // TL
                case 1: newMax.x += worldDx; newMax.y += worldDy; break; // TR
                case 2: newMax.x += worldDx; newMin.y += worldDy; break; // BR
                case 3: newMin.x += worldDx; newMin.y += worldDy; break; // BL
            }

            if (newMax.x - newMin.x < MIN_REGION_SIZE ||
                newMax.y - newMin.y < MIN_REGION_SIZE)
                return;

            if (region.parent)
            {
                auto pPts = region.parent->geometry.getPoints();
                double pMinX = pPts[0].x, pMaxX = pPts[0].x;
                double pMinY = pPts[0].y, pMaxY = pPts[0].y;
                for (const auto& p : pPts)
                {
                    pMinX = std::min(pMinX, p.x); pMaxX = std::max(pMaxX, p.x);
                    pMinY = std::min(pMinY, p.y); pMaxY = std::max(pMaxY, p.y);
                }
                if (newMin.x < pMinX || newMax.x > pMaxX ||
                    newMin.y < pMinY || newMax.y > pMaxY)
                    return;
            }

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

            Vec2 newPt { pts[idx].x + worldDx, pts[idx].y + worldDy };

            if (region.parent && !region.parent->geometry.contains(newPt))
                return;

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
        RegionSerializer::save(regionTree, "regions.json");
    }
}

// ---------------------------------------------------------------
// Delete polygon point in edit mode
// Called from UILayer when [Del] is pressed while edit tool is active
// and a polygon point handle is selected.
// Connects the two neighbours and removes the point.
// Minimum 3 points enforced.
// ---------------------------------------------------------------

void Core::deleteEditPoint()
{
    if (!editState.isActive()) return;
    if (editState.handleType != EditHandleType::PolyPoint) return;

    Region& region = *editState.target;
    auto& pts = region.geometry.points;
    int n = static_cast<int>(pts.size());
    int idx = editState.handleIndex;

    if (idx < 0 || idx >= n) return;

    // Need at least 3 points after deletion to remain a valid polygon
    if (n <= 3) return;

    pts.erase(pts.begin() + idx);

    // Clamp handleIndex so it stays valid after erasure
    editState.handleIndex = std::min(editState.handleIndex,
                                     static_cast<int>(pts.size()) - 1);

    RegionSerializer::save(regionTree, "regions.json");
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
