#pragma once

#include <vector>
#include "math/Vec2.h"

enum class GeometryType
{
    Rectangle,
    Polygon
};

struct RegionGeometry
{
    GeometryType type = GeometryType::Rectangle;

    Vec2 rectMin { 0.0, 0.0 };
    Vec2 rectMax { 0.0, 0.0 };

    std::vector<Vec2> points;

    // --- Helpers ---

    bool isValid() const
    {
        if (type == GeometryType::Rectangle)
            return rectMax.x > rectMin.x && rectMax.y > rectMin.y;

        return points.size() >= 3;
    }

    std::vector<Vec2> getPoints() const
    {
        if (type == GeometryType::Polygon)
            return points;

        return {
            { rectMin.x, rectMin.y },
            { rectMax.x, rectMin.y },
            { rectMax.x, rectMax.y },
            { rectMin.x, rectMax.y }
        };
    }

    // Returns true if the world-space point is inside this geometry.
    // Uses ray-casting algorithm — works for both convex and concave polygons.
    bool contains(const Vec2& p) const
    {
        std::vector<Vec2> pts = getPoints();
        if (pts.size() < 3) return false;

        bool inside = false;
        int n = static_cast<int>(pts.size());

        for (int i = 0, j = n - 1; i < n; j = i++)
        {
            const Vec2& a = pts[i];
            const Vec2& b = pts[j];

            bool crosses =
                ((a.y > p.y) != (b.y > p.y)) &&
                (p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x);

            if (crosses)
                inside = !inside;
        }

        return inside;
    }
};
