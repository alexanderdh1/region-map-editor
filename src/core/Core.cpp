#include "core/Core.h"
#include "input/Input.h"
#include "math/Vec2.h"
#include "data/Region.h"
#include "data/RegionSerializer.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <memory>
#include <cmath>

// Minimum side length in world-units for a new region.
// Prevents unclickable 1-pixel-wide regions from being created.
static constexpr double MIN_REGION_SIZE = 4.0;

Core::Core() = default;

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

    // ZOOM — zoom toward mouse cursor position
    if (input.hasZoomDelta()) {
        double zoomSteps = input.consumeZoomDelta();
        const double zoomFactor = 1.1;

        Vec2 worldBefore = camera.screenToWorld(mouseScreen);

        if (zoomSteps > 0)
            camera.zoomBy(zoomFactor);
        else
            camera.zoomBy(1.0 / zoomFactor);

        Vec2 worldAfter = camera.screenToWorld(mouseScreen);

        camera.position.x += (worldBefore.x - worldAfter.x);
        camera.position.y += (worldBefore.y - worldAfter.y);
    }

    // Enforce zoom limits and keep camera within world bounds
    camera.clampToBounds(worldWidth, worldHeight);

    // Helper to add region — respects pending parent, then auto-saves
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

    // Helper: check if a world point is inside the pending parent region
    auto isInsideParent = [&](const Vec2& worldPt) -> bool
    {
        if (!hasPendingParent) return true; // no restriction
        Region* parent = regionTree.findById(pendingParentId);
        if (!parent) return true;
        return parent->geometry.contains(worldPt);
    };

    // RECT — capture world-space start the moment drawing begins
    if (input.isRectJustStarted())
    {
        Vec2 worldStart = camera.screenToWorld(input.getDrawStart());
        // Cancel immediately if start point is outside parent
        if (!isInsideParent(worldStart))
        {
            input.cancelRect();
        }
        else
        {
            input.setDrawStartWorld(worldStart);
            input.consumeRectJustStarted();
        }
    }

    // RECT — finalise when user releases
    if (input.hasCompletedRect())
    {
        Vec2 worldA = input.getDrawStartWorld();

        // Clamp current to viewport before converting to world space
        Vec2 cur = input.getDrawCurrent();
        cur.x = std::max(0.0, std::min(cur.x, camera.viewportSize.x));
        cur.y = std::max(0.0, std::min(cur.y, camera.viewportSize.y));
        Vec2 worldB = camera.screenToWorld(cur);

        // Clamp worldB to parent bounds if needed
        if (hasPendingParent)
        {
            Region* parent = regionTree.findById(pendingParentId);
            if (parent)
            {
                const auto& pts = parent->geometry.getPoints();
                // Simple AABB clamp using parent bounding box
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
        geom.rectMin = { std::min(worldA.x, worldB.x),
                         std::min(worldA.y, worldB.y) };
        geom.rectMax = { std::max(worldA.x, worldB.x),
                         std::max(worldA.y, worldB.y) };

        // Enforce minimum size — reject regions too small to click
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

    // POLYGON — add world-space point only if inside parent
    if (input.hasPendingPolyPoint())
    {
        Vec2 screenPt = input.consumePendingPolyPoint();
        Vec2 worldPt  = camera.screenToWorld(screenPt);

        // Reject point if outside parent
        if (!isInsideParent(worldPt))
        {
            // discard — do not add
        }
        else
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

    // POLYGON — finalise when double-click or first-point click detected
    if (input.hasCompletedPolygon())
    {
        std::vector<Vec2> worldPoints = input.consumeCompletedPolygon();

        RegionGeometry geom;
        geom.type   = GeometryType::Polygon;
        geom.points = worldPoints;

        // Enforce minimum bounding box size for polygons
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

            bool tooSmall =
                (maxX - minX) < MIN_REGION_SIZE &&
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

    // CLICK — hit-test regions, select or deselect
    if (input.hasClick())
    {
        Vec2 screenPos = input.consumeClick();
        Vec2 worldPos  = camera.screenToWorld(screenPos);

        // Find topmost region under cursor
        Region* hit = nullptr;
        regionTree.forEach([&](Region& r)
        {
            if (r.geometry.contains(worldPos))
                hit = &r;
        });

        if (hit)
        {
            // Build the ancestor chain for viewStack so [B] always works
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

    // RIGHT-CLICK — hit-test for context menu (handled by UILayer)
    if (input.hasRightClick())
    {
        Vec2 screenPos = input.consumeRightClick();
        Vec2 worldPos  = camera.screenToWorld(screenPos);

        Region* hit = nullptr;
        regionTree.forEach([&](Region& r)
        {
            if (r.geometry.contains(worldPos))
                hit = &r;
        });

        selection.contextRegion     = hit;
        selection.contextMenuOpen   = (hit != nullptr);
        selection.contextMenuScreen = screenPos;
    }
}

Camera& Core::getCamera() {
    return camera;
}

const Camera& Core::getCamera() const {
    return camera;
}

Input& Core::getInput() {
    return input;
}

const Input& Core::getInput() const {
    return input;
}

void Core::setWorldSize(double width, double height)
{
    worldWidth  = width;
    worldHeight = height;

    double minZoomX = camera.viewportSize.x / worldWidth;
    double minZoomY = camera.viewportSize.y / worldHeight;

    camera.minZoom = std::max(minZoomX, minZoomY);
}

void Core::setWorldBlockBounds(
    int minBlockX,
    int minBlockZ,
    int maxBlockX,
    int maxBlockZ
)
{
    worldMinBlockX = minBlockX;
    worldMinBlockZ = minBlockZ;
    worldMaxBlockX = maxBlockX;
    worldMaxBlockZ = maxBlockZ;
}

Vec2 Core::blockToWorld(const BlockCoord& block) const
{
    double worldX = block.x - worldMinBlockX;

    // Flip Z
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

    // Flip Z
    int blockZ = worldMaxBlockZ - static_cast<int>(localZ);

    return { blockX, blockZ };
}
