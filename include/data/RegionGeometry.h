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

    // Returns true if any two non-adjacent edges intersect,
    // OR if any vertex lies inside the polygon formed by the other vertices.
    bool isSelfIntersecting() const
    {
        if (type == GeometryType::Rectangle) return false;
        if (points.size() < 4) return false;

        int n = static_cast<int>(points.size());

        // --- Edge-edge intersection test ---
        auto segmentsIntersect = [](
            const Vec2& a, const Vec2& b,
            const Vec2& c, const Vec2& d) -> bool
        {
            auto cross = [](const Vec2& o, const Vec2& p, const Vec2& q) {
                return (p.x - o.x) * (q.y - o.y)
                     - (p.y - o.y) * (q.x - o.x);
            };

            double d1 = cross(c, d, a);
            double d2 = cross(c, d, b);
            double d3 = cross(a, b, c);
            double d4 = cross(a, b, d);

            return ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
                   ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0));
        };

        for (int i = 0; i < n; i++)
        {
            const Vec2& a = points[i];
            const Vec2& b = points[(i + 1) % n];

            for (int j = i + 2; j < n; j++)
            {
                if (i == 0 && j == n - 1) continue;

                const Vec2& c = points[j];
                const Vec2& d = points[(j + 1) % n];

                if (segmentsIntersect(a, b, c, d))
                    return true;
            }
        }

        // --- Interior point test ---
        // Check if any vertex lies strictly inside the polygon
        // formed by all OTHER vertices (using winding number).
        auto windingNumber = [](const Vec2& p, const std::vector<Vec2>& poly) -> int
        {
            int winding = 0;
            int m = static_cast<int>(poly.size());
            for (int i = 0, j = m - 1; i < m; j = i++)
            {
                const Vec2& a = poly[j];
                const Vec2& b = poly[i];
                if (a.y <= p.y)
                {
                    if (b.y > p.y)
                    {
                        double cross = (b.x - a.x) * (p.y - a.y)
                                     - (p.x - a.x) * (b.y - a.y);
                        if (cross > 0) ++winding;
                    }
                }
                else
                {
                    if (b.y <= p.y)
                    {
                        double cross = (b.x - a.x) * (p.y - a.y)
                                     - (p.x - a.x) * (b.y - a.y);
                        if (cross < 0) --winding;
                    }
                }
            }
            return winding;
        };

        for (int i = 0; i < n; i++)
        {
            // Build polygon without point i
            std::vector<Vec2> others;
            others.reserve(n - 1);
            for (int j = 0; j < n; j++)
                if (j != i) others.push_back(points[j]);

            if (windingNumber(points[i], others) != 0)
                return true;
        }

        return false;
    }
    // Uses the nonzero winding rule — self-intersecting polygons treated as inside.
    // A tiny jitter avoids degenerate edge-hits when regions share exact boundaries.
    bool contains(const Vec2& p) const
    {
        std::vector<Vec2> pts = getPoints();
        if (pts.size() < 3) return false;

        // Slight offset to avoid landing exactly on shared edges
        const Vec2 tp { p.x + 0.0013, p.y + 0.0009 };

        int winding = 0;
        int n = static_cast<int>(pts.size());

        for (int i = 0, j = n - 1; i < n; j = i++)
        {
            const Vec2& a = pts[j];
            const Vec2& b = pts[i];

            if (a.y <= tp.y)
            {
                if (b.y > tp.y)
                {
                    double cross = (b.x - a.x) * (tp.y - a.y)
                                 - (tp.x - a.x) * (b.y - a.y);
                    if (cross > 0) ++winding;
                }
            }
            else
            {
                if (b.y <= tp.y)
                {
                    double cross = (b.x - a.x) * (tp.y - a.y)
                                 - (tp.x - a.x) * (b.y - a.y);
                    if (cross < 0) --winding;
                }
            }
        }

        return winding != 0;
    }
};
