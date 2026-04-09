#include "data/RegionSerializer.h"

#include "data/Region.h"
#include "data/RegionGeometry.h"
#include "core/Core.h"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ============================================================
// Coordinate conversion helpers
// ============================================================

// World → normalised (0.0–1.0 relative to image size)
static Vec2 worldToNorm(const Vec2& w, double imgW, double imgH)
{
    // World-space is centred at (0,0); top-left is (-imgW/2, imgH/2)
    return {
        (w.x + imgW / 2.0) / imgW,
        (imgH / 2.0 - w.y) / imgH   // Y is flipped: world +Y = image up
    };
}

// Normalised → world
static Vec2 normToWorld(const Vec2& n, double imgW, double imgH)
{
    return {
        n.x * imgW - imgW / 2.0,
        imgH / 2.0 - n.y * imgH
    };
}

// ============================================================
// JSON helpers
// ============================================================

static json serializeVec2(const Vec2& v)       { return { v.x, v.y }; }
static Vec2 deserializeVec2(const json& j)     { return { j[0].get<double>(), j[1].get<double>() }; }

// Serialize geometry using the correct coordinate representation
static json serializeGeometry(const RegionGeometry& g, const Core& core)
{
    json j;
    j["type"] = (g.type == GeometryType::Rectangle) ? "Rectangle" : "Polygon";

    auto convertPt = [&](const Vec2& world) -> json
    {
        if (core.isCoordMode())
        {
            MapCoord b = core.worldToCoord(world);
            return { b.x, b.y };
        }
        else
        {
            Vec2 n = worldToNorm(world, core.getMapWidth(), core.getMapHeight());
            return serializeVec2(n);
        }
    };

    if (g.type == GeometryType::Rectangle)
    {
        j["rectMin"] = convertPt(g.rectMin);
        j["rectMax"] = convertPt(g.rectMax);
    }
    else
    {
        json pts = json::array();
        for (const Vec2& p : g.points)
            pts.push_back(convertPt(p));
        j["points"] = pts;
    }
    return j;
}

static RegionGeometry deserializeGeometry(const json& j, const Core& core)
{
    RegionGeometry g;
    std::string type = j.at("type").get<std::string>();

    auto convertPt = [&](const json& pt) -> Vec2
    {
        if (core.isCoordMode())
        {
            MapCoord b { pt[0].get<int>(), pt[1].get<int>() };
            return core.coordToWorld(b);
        }
        else
        {
            Vec2 n = deserializeVec2(pt);
            return normToWorld(n, core.getMapWidth(), core.getMapHeight());
        }
    };

    if (type == "Rectangle")
    {
        g.type    = GeometryType::Rectangle;
        g.rectMin = convertPt(j.at("rectMin"));
        g.rectMax = convertPt(j.at("rectMax"));
    }
    else
    {
        g.type = GeometryType::Polygon;
        for (const auto& p : j.at("points"))
            g.points.push_back(convertPt(p));
    }
    return g;
}

// ============================================================
// Region serialization
// ============================================================

static json serializeRegion(const Region& r, const Core& core)
{
    json j;
    j["id"]        = r.id;
    j["name"]      = r.name;
    j["note"]      = r.note;
    j["color"]     = { r.colorR, r.colorG, r.colorB, r.colorA };
    j["hidden"]    = r.hidden;
    j["collapsed"] = r.collapsed;
    j["geometry"]  = serializeGeometry(r.geometry, core);

    json children = json::array();
    for (const auto& child : r.children)
        children.push_back(serializeRegion(*child, core));
    j["children"] = children;
    return j;
}

static std::unique_ptr<Region> deserializeRegion(
    const json& j, RegionTree& tree, const Core& core)
{
    auto r      = std::make_unique<Region>();
    r->id       = j.at("id").get<RegionId>();
    r->name     = j.at("name").get<std::string>();
    r->note     = j.at("note").get<std::string>();

    const auto& c = j.at("color");
    r->colorR = c[0].get<float>();
    r->colorG = c[1].get<float>();
    r->colorB = c[2].get<float>();
    r->colorA = c[3].get<float>();

    r->hidden    = j.value("hidden",    false);
    r->collapsed = j.value("collapsed", false);
    r->geometry  = deserializeGeometry(j.at("geometry"), core);

    tree.ensureNextIdAbove(r->id);

    for (const auto& childJson : j.at("children"))
    {
        auto child = deserializeRegion(childJson, tree, core);
        r->addChild(std::move(child));
    }
    return r;
}

// ============================================================
// Public API
// ============================================================

bool RegionSerializer::save(const RegionTree& tree,
                             const std::string& path,
                             const Core& core)
{
    json root;
    root["version"]    = 3;
    root["coord_mode"] = core.isCoordMode() ? "coord" : "normalised";

    json regions = json::array();
    for (const auto& r : tree.roots())
        regions.push_back(serializeRegion(*r, core));
    root["regions"] = regions;

    std::ofstream file(path);
    if (!file)
    {
        std::cerr << "[RegionSerializer] Could not open file for writing: " << path << "\n";
        return false;
    }

    file << root.dump(2);
    return true;
}

bool RegionSerializer::load(RegionTree& tree,
                             const std::string& path,
                             const Core& core)
{
    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "[RegionSerializer] Could not open file for reading: " << path << "\n";
        return false;
    }

    // Empty file is valid — nothing to load
    file.seekg(0, std::ios::end);
    if (file.tellg() == 0)
        return true;
    file.seekg(0, std::ios::beg);

    json root;
    try { file >> root; }
    catch (const std::exception& e)
    {
        std::cerr << "[RegionSerializer] JSON parse error: " << e.what() << "\n";
        return false;
    }

    // Warn if coordinate mode mismatch between file and current world
    if (root.contains("coord_mode"))
    {
        std::string fileMode = root["coord_mode"].get<std::string>();
        std::string currMode = core.isCoordMode() ? "coord" : "normalised";
        if (fileMode != currMode)
        {
            std::cerr << "[RegionSerializer] Warning: file was saved in '"
                      << fileMode << "' mode but current world is '"
                      << currMode << "' mode. Coordinates may be incorrect.\n";
        }
    }

    while (!tree.roots().empty())
        tree.removeRegion(tree.roots().front()->id);

    int count = 0;
    for (const auto& regionJson : root.at("regions"))
    {
        auto region = deserializeRegion(regionJson, tree, core);
        tree.addRegion(std::move(region));
        count++;
    }

    return true;
}
