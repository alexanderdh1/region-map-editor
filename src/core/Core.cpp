#include "core/Core.h"
#include "input/Input.h"
#include "math/Vec2.h"
#include "data/Region.h"
#include "data/RegionSerializer.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <memory>
#include <cmath>

static constexpr double MIN_REGION_SIZE = 4.0;
static constexpr double HANDLE_RADIUS_PX = 12.0;
static constexpr double EDGE_HIT_PX = 8.0;

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
        Vec2 mapBefore = cam.screenToWorld(mouseScreen);
        if (zoomSteps > 0) cam.zoomBy(zoomFactor);
        else               cam.zoomBy(1.0 / zoomFactor);
        Vec2 mapAfter = cam.screenToWorld(mouseScreen);
        cam.position.x += (mapBefore.x - mapAfter.x);
        cam.position.y += (mapBefore.y - mapAfter.y);
    }

    cam.clampToBounds(mapWidth, mapHeight);

    // Hover is recomputed from scratch every frame (edit mode has its own highlight)
    selection.hoveredRegion = nullptr;

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

    // HOVER — topmost visible region under the cursor (same rule as click)
    {
        Vec2 mouseMap = cam.screenToWorld(mouseScreen);
        regionTree.forEach([&](Region& r)
        {
            if (!r.hidden && r.geometry.contains(mouseMap))
                selection.hoveredRegion = &r;
        });
    }

    auto addRegion = [&](std::unique_ptr<Region> region)
    {
        pushSnapshot();
        if (hasPendingParent)
        {
            RegionId newId   = region->id;
            RegionId parentId = pendingParentId;
            regionTree.addChildRegion(parentId, std::move(region));
            hasPendingParent = false;

            Region* added = regionTree.findById(newId);
            if (added)
            {
                selection.viewStack.clear();
                Region* anc = added->parent;
                while (anc) { selection.viewStack.insert(selection.viewStack.begin(), anc); anc = anc->parent; }
                selection.selectedRegion = added;
                selection.popupOpen      = true;
            }
        }
        else
        {
            regionTree.addRegion(std::move(region));
        }
        RegionSerializer::save(regionTree, "regions.json", *this);
    };

    // Clamp a point into the pending parent: clicking just outside the
    // parent snaps onto its border instead of being rejected, so child
    // regions can hug the parent's edge without pixel-perfect clicks.
    auto clampToParent = [&](const Vec2& mapPt) -> Vec2
    {
        if (!hasPendingParent) return mapPt;
        Region* parent = regionTree.findById(pendingParentId);
        if (!parent) return mapPt;
        return parent->geometry.closestPointInside(mapPt);
    };

    // Auto sub-region: if a region is selected and drawing starts inside it,
    // automatically treat the new shape as a child — no right-click needed.
    auto autoSetPendingParent = [&](const Vec2& mapPt)
    {
        if (hasPendingParent) return; // already set explicitly
        Region* selected = selection.selectedRegion;
        if (!selected) return;
        if (selected->geometry.contains(mapPt))
        {
            pendingParentId  = selected->id;
            hasPendingParent = true;
        }
    };

    // RECT
    if (input.isRectJustStarted())
    {
        Vec2 mapStart = cam.screenToWorld(input.getDrawStart());
        autoSetPendingParent(mapStart);
        input.setDrawStartMap(clampToParent(mapStart));
        input.consumeRectJustStarted();
    }

    if (input.hasCompletedRect())
    {
        Vec2 mapA = input.getDrawStartMap();
        Vec2 cur = input.getDrawCurrent();
        cur.x = std::max(0.0, std::min(cur.x, cam.viewportSize.x));
        cur.y = std::max(0.0, std::min(cur.y, cam.viewportSize.y));
        Vec2 mapB = cam.screenToWorld(cur);

        mapB = clampToParent(mapB);

        RegionGeometry geom;
        geom.type    = GeometryType::Rectangle;
        geom.rectMin = { std::min(mapA.x, mapB.x), std::min(mapA.y, mapB.y) };
        geom.rectMax = { std::max(mapA.x, mapB.x), std::max(mapA.y, mapB.y) };

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
        Vec2 mapPt  = cam.screenToWorld(screenPt);

        // Auto sub-region on first polygon point
        if (input.getPolygonMapPoints().empty())
            autoSetPendingParent(mapPt);

        // Snap onto the parent border when clicking outside it
        mapPt = clampToParent(mapPt);

        const std::vector<Vec2>& existing = input.getPolygonMapPoints();
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
        {
            // If the new edge would cut across outside the parent (two
            // snapped points around a parent corner), insert the parent's
            // corner points so the child follows the parent border.
            if (hasPendingParent && !existing.empty())
            {
                Region* parent = regionTree.findById(pendingParentId);
                Vec2 last = existing.back();
                if (parent && !parent->geometry.segmentInside(last, mapPt))
                {
                    for (const Vec2& v :
                         parent->geometry.boundaryPathBetween(last, mapPt))
                        input.addPolygonMapPoint(v);
                }
            }
            input.addPolygonMapPoint(mapPt);
        }
    }

    if (input.hasCompletedPolygon())
    {
        std::vector<Vec2> mapPoints = input.consumeCompletedPolygon();

        // The closing edge (last → first) can also cut outside the parent —
        // append the parent border path before validating.
        if (hasPendingParent && mapPoints.size() >= 3)
        {
            Region* parent = regionTree.findById(pendingParentId);
            if (parent &&
                !parent->geometry.segmentInside(mapPoints.back(), mapPoints.front()))
            {
                std::vector<Vec2> path = parent->geometry.boundaryPathBetween(
                    mapPoints.back(), mapPoints.front());
                mapPoints.insert(mapPoints.end(), path.begin(), path.end());
            }
        }

        RegionGeometry geom;
        geom.type   = GeometryType::Polygon;
        geom.points = mapPoints;

        if (geom.isValid() && !geom.isSelfIntersecting())
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
        Vec2 mapPos  = cam.screenToWorld(screenPos);

        Region* hit = nullptr;
        regionTree.forEach([&](Region& r)
        {
            if (!r.hidden && r.geometry.contains(mapPos)) hit = &r;
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

    // DOUBLE-CLICK — zoom to the region under the cursor
    if (input.hasDoubleClick())
    {
        Vec2 screenPos = input.consumeDoubleClick();
        Vec2 mapPos  = cam.screenToWorld(screenPos);

        Region* hit = nullptr;
        regionTree.forEach([&](Region& r)
        {
            if (!r.hidden && r.geometry.contains(mapPos)) hit = &r;
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
            zoomToRegion(*hit);
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

    // Middle-mouse pan works in edit mode too — apply whatever arrived.
    if (input.hasPanDelta())
        cam.panBy(input.consumePanDelta());

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
            Vec2 mapCorners[4] = {
                { hoverRegion.geometry.rectMin.x, hoverRegion.geometry.rectMax.y },
                { hoverRegion.geometry.rectMax.x, hoverRegion.geometry.rectMax.y },
                { hoverRegion.geometry.rectMax.x, hoverRegion.geometry.rectMin.y },
                { hoverRegion.geometry.rectMin.x, hoverRegion.geometry.rectMin.y },
            };
            for (int i = 0; i < 4; i++)
            {
                Vec2 s = cam.worldToScreen(mapCorners[i]);
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
        Vec2 clickMapPos  = cam.screenToWorld(clickScreen);

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
                pushSnapshot();
                pts.insert(pts.begin() + bestEdge + 1, clickMapPos);
                RegionSerializer::save(regionTree, "regions.json", *this);
            }
            else
            {
                // Click outside region geometry and all edges — deselect handle
                bool outsideGeometry = !region.geometry.contains(clickMapPos);
                bool outsideEdges    = true;
                for (int i = 0; i < n && outsideEdges; i++)
                {
                    int j = (i + 1) % n;
                    Vec2 sA = cam.worldToScreen(pts[i]);
                    Vec2 sB = cam.worldToScreen(pts[j]);
                    Vec2 ab { sB.x - sA.x, sB.y - sA.y };
                    Vec2 ap { clickScreen.x - sA.x, clickScreen.y - sA.y };
                    double lenSq = ab.x * ab.x + ab.y * ab.y;
                    double t = (lenSq > 0.0)
                        ? std::clamp((ap.x * ab.x + ap.y * ab.y) / lenSq, 0.0, 1.0)
                        : 0.0;
                    Vec2 closest { sA.x + t * ab.x, sA.y + t * ab.y };
                    double dist = std::hypot(
                        clickScreen.x - closest.x,
                        clickScreen.y - closest.y);
                    if (dist <= EDGE_HIT_PX)
                        outsideEdges = false;
                }
                if (outsideGeometry && outsideEdges)
                    editState.clearDrag();
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
            // Corner map positions: TL, TR, BR, BL
            Vec2 mapCorners[4] = {
                { region.geometry.rectMin.x, region.geometry.rectMax.y },
                { region.geometry.rectMax.x, region.geometry.rectMax.y },
                { region.geometry.rectMax.x, region.geometry.rectMin.y },
                { region.geometry.rectMin.x, region.geometry.rectMin.y },
            };
            for (int i = 0; i < 4; i++)
            {
                Vec2 s = cam.worldToScreen(mapCorners[i]);
                double dist = std::hypot(startScreen.x - s.x, startScreen.y - s.y);
                if (dist <= HANDLE_RADIUS_PX)
                {
                    editState.handleType     = EditHandleType::RectCorner;
                    editState.handleIndex    = i;
                    // Store the map position of this corner at drag start
                    editState.dragOriginMap = mapCorners[i];
                    pushSnapshot();
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
                    // Store the map position of this point at drag start
                    editState.dragOriginMap = pts[i];
                    pushSnapshot();
                    return;
                }
            }
        }

        // No handle was hit — discard the press (panning is middle-mouse)
        input.cancelEditPress();
    }

    // ---- Dragging ----
    // Use ABSOLUTE positioning: new position = dragOriginMap + totalMouseDelta/zoom.
    // This means constraints never cause offset accumulation — if the mouse moves
    // 1000px past a constraint boundary and reverses, the geometry only starts
    // moving again exactly when the mouse returns to the constraint point.
    if (input.isEditDragging())
    {
        // Per-frame delta must always be consumed to prevent stale accumulation
        input.getEditDragDelta();

        if (!editState.isDragging()) return;

        // Convert total mouse movement to map space
        Vec2 totalScreen = input.getEditDragTotalDelta();
        double mapDx =  totalScreen.x / cam.zoom;
        double mapDy = -totalScreen.y / cam.zoom;

        // Desired absolute map position of the dragged point
        Vec2 desiredMapPos {
            editState.dragOriginMap.x + mapDx,
            editState.dragOriginMap.y + mapDy
        };

        Region& region = *editState.target;

        if (editState.handleType == EditHandleType::RectCorner)
        {
            // Snap the dragged corner onto the parent border instead of
            // freezing the drag the moment the mouse leaves the parent.
            if (region.parent)
                desiredMapPos = region.parent->geometry.closestPointInside(desiredMapPos);

            Vec2 newMin = region.geometry.rectMin;
            Vec2 newMax = region.geometry.rectMax;

            switch (editState.handleIndex)
            {
                case 0: newMin.x = desiredMapPos.x; newMax.y = desiredMapPos.y; break; // TL
                case 1: newMax.x = desiredMapPos.x; newMax.y = desiredMapPos.y; break; // TR
                case 2: newMax.x = desiredMapPos.x; newMin.y = desiredMapPos.y; break; // BR
                case 3: newMin.x = desiredMapPos.x; newMin.y = desiredMapPos.y; break; // BL
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

            Vec2 newPt = desiredMapPos;

            // Parent containment: snap onto the parent border so the point
            // slides along the edge instead of freezing outside it
            if (region.parent)
                newPt = region.parent->geometry.closestPointInside(newPt);

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

    // ---- Drag end: repair parent containment, then auto-save ----
    if (input.hasEditDragEnd())
    {
        input.consumeEditDragEnd();

        // After dragging a polygon point along the parent border, the two
        // edges adjacent to it can cut across outside the parent (concave
        // corners). Insert the parent border path on release — same fix as
        // when drawing, but deferred so points don't spawn mid-drag.
        if (editState.isActive() &&
            editState.handleType == EditHandleType::PolyPoint &&
            editState.target->parent &&
            editState.target->geometry.type == GeometryType::Polygon)
        {
            Region& region = *editState.target;
            const RegionGeometry& pg = region.parent->geometry;
            auto& pts = region.geometry.points;
            int idx = editState.handleIndex;
            int n   = static_cast<int>(pts.size());

            if (idx >= 0 && idx < n && n >= 3)
            {
                // Edge dragged → next: insert after idx (doesn't shift idx)
                int next = (idx + 1) % n;
                if (!pg.segmentInside(pts[idx], pts[next]))
                {
                    std::vector<Vec2> path =
                        pg.boundaryPathBetween(pts[idx], pts[next]);
                    pts.insert(pts.begin() + idx + 1, path.begin(), path.end());
                }

                // Edge prev → dragged: insert before idx, keep handle on the
                // dragged point by shifting its index
                n = static_cast<int>(pts.size());
                int prev = (idx - 1 + n) % n;
                if (!pg.segmentInside(pts[prev], pts[idx]))
                {
                    std::vector<Vec2> path =
                        pg.boundaryPathBetween(pts[prev], pts[idx]);
                    pts.insert(pts.begin() + idx, path.begin(), path.end());
                    editState.handleIndex = idx + static_cast<int>(path.size());
                }
            }
        }

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

    pushSnapshot();
    pts.erase(pts.begin() + idx);
    // Clear the entire drag state so the dragging segment that runs later
    // in the same frame cannot write through a now-stale handleIndex.
    editState.clearDrag();

    RegionSerializer::save(regionTree, "regions.json", *this);
}

// ---------------------------------------------------------------
// Zoom to region
// ---------------------------------------------------------------

void Core::zoomToRegion(const Region& region)
{
    if (!region.geometry.isValid()) return;

    double minX, minY, maxX, maxY;
    regionBoundingBox(region, minX, minY, maxX, maxY);

    double w = maxX - minX;
    double h = maxY - minY;
    if (w <= 0.0 || h <= 0.0) return;

    // Fit with a margin so the region doesn't touch the window edges
    double zoomX = camera.viewportSize.x / w;
    double zoomY = camera.viewportSize.y / h;
    double zoom  = std::min(zoomX, zoomY) * 0.80;

    camera.zoom     = std::min(zoom, camera.maxZoom);
    camera.position = { (minX + maxX) / 2.0, (minY + maxY) / 2.0 };
    camera.clampToBounds(mapWidth, mapHeight);
}

// ---------------------------------------------------------------
// Undo / redo
// ---------------------------------------------------------------

void Core::pushSnapshot()
{
    history_.pushSnapshot(regionTree, *this);
}

bool Core::undo()
{
    if (!history_.canUndo()) return false;
    history_.undo(regionTree, *this);
    selection.clear();
    editState.clear();
    input.cancelEdit();
    input.cancelPolygon();
    input.cancelRect();
    input.setDrawTool(DrawTool::Navigate);
    hasPendingParent = false;
    RegionSerializer::save(regionTree, "regions.json", *this);
    return true;
}

bool Core::redo()
{
    if (!history_.canRedo()) return false;
    history_.redo(regionTree, *this);
    selection.clear();
    editState.clear();
    input.cancelEdit();
    input.cancelPolygon();
    input.cancelRect();
    input.setDrawTool(DrawTool::Navigate);
    hasPendingParent = false;
    RegionSerializer::save(regionTree, "regions.json", *this);
    return true;
}

// ---------------------------------------------------------------
// Camera / map helpers
// ---------------------------------------------------------------

Camera& Core::getCamera() { return camera; }
const Camera& Core::getCamera() const { return camera; }
Input& Core::getInput() { return input; }
const Input& Core::getInput() const { return input; }

void Core::setMapSize(double width, double height)
{
    mapWidth  = width;
    mapHeight = height;
    double minZoomX = camera.viewportSize.x / mapWidth;
    double minZoomY = camera.viewportSize.y / mapHeight;
    camera.minZoom = std::max(minZoomX, minZoomY);
}

void Core::setMapCoordBounds(
    int minX, int minY,
    int maxX, int maxY)
{
    mapMinX = minX;
    mapMinY = minY;
    mapMaxX = maxX;
    mapMaxY = maxY;
}

Vec2 Core::coordToMap(const MapCoord& coord) const
{
    double mapX = coord.x - mapMinX;
    double mapY = mapMaxY - coord.y;
    mapX -= mapWidth  / 2.0;
    mapY -= mapHeight / 2.0;
    return { mapX, mapY };
}

MapCoord Core::mapToCoord(const Vec2& mapPos) const
{
    double localX = mapPos.x + mapWidth  / 2.0;
    double localY = mapPos.y + mapHeight / 2.0;
    int coordX = static_cast<int>(localX) + mapMinX;
    int coordY = mapMaxY - static_cast<int>(localY);
    return { coordX, coordY };
}
