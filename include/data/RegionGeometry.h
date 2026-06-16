#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
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

    // Index of the boundary edge closest to p (edge i runs pts[i] → pts[i+1]).
    // Optionally outputs the closest point on that edge. Returns -1 if degenerate.
    int closestBoundaryEdge(const Vec2& p, Vec2* closestOut = nullptr) const
    {
        std::vector<Vec2> pts = getPoints();
        int n = static_cast<int>(pts.size());
        if (n < 3) return -1;

        double bestDistSq = -1.0;
        Vec2   best { 0.0, 0.0 };
        int    bestEdge = -1;

        for (int i = 0; i < n; i++)
        {
            const Vec2& a = pts[i];
            const Vec2& b = pts[(i + 1) % n];

            Vec2 ab { b.x - a.x, b.y - a.y };
            Vec2 ap { p.x - a.x, p.y - a.y };
            double lenSq = ab.x * ab.x + ab.y * ab.y;
            double t = (lenSq > 0.0)
                ? std::clamp((ap.x * ab.x + ap.y * ab.y) / lenSq, 0.0, 1.0)
                : 0.0;
            Vec2 c { a.x + t * ab.x, a.y + t * ab.y };
            double dSq = (p.x - c.x) * (p.x - c.x) + (p.y - c.y) * (p.y - c.y);

            if (bestDistSq < 0.0 || dSq < bestDistSq)
            {
                bestDistSq = dSq;
                best       = c;
                bestEdge   = i;
            }
        }

        if (closestOut) *closestOut = best;
        return bestEdge;
    }

    // Signed polygon area — positive when wound counter-clockwise.
    double signedArea() const
    {
        std::vector<Vec2> pts = getPoints();
        int n = static_cast<int>(pts.size());
        double area = 0.0;
        for (int i = 0; i < n; i++)
        {
            const Vec2& a = pts[i];
            const Vec2& b = pts[(i + 1) % n];
            area += a.x * b.y - b.x * a.y;
        }
        return area;
    }

    // Vertex idx pushed slightly into the interior (along the bisector of
    // the two adjacent edges' inward normals) so contains() passes for it.
    Vec2 nudgedVertex(int idx) const
    {
        std::vector<Vec2> pts = getPoints();
        int n = static_cast<int>(pts.size());
        if (n < 3 || idx < 0 || idx >= n) return pts.empty() ? Vec2{0,0} : pts[0];

        const double eps = 0.05;
        bool ccw = signedArea() > 0.0;

        auto inwardNormal = [&](const Vec2& a, const Vec2& b) -> Vec2
        {
            Vec2 d { b.x - a.x, b.y - a.y };
            double len = std::hypot(d.x, d.y);
            if (len <= 0.0) return { 0.0, 0.0 };
            return ccw ? Vec2{ -d.y / len,  d.x / len }
                       : Vec2{  d.y / len, -d.x / len };
        };

        const Vec2& prev = pts[(idx - 1 + n) % n];
        const Vec2& v    = pts[idx];
        const Vec2& next = pts[(idx + 1) % n];

        Vec2 n1 = inwardNormal(prev, v);
        Vec2 n2 = inwardNormal(v, next);
        Vec2 m  { n1.x + n2.x, n1.y + n2.y };
        double ml = std::hypot(m.x, m.y);
        if (ml <= 0.0) return v;

        Vec2 q { v.x + m.x / ml * eps, v.y + m.y / ml * eps };
        if (!contains(q)) // reflex corner — try the opposite direction
            q = { v.x - m.x / ml * eps, v.y - m.y / ml * eps };
        return q;
    }

    // Returns p unchanged when it is inside; otherwise the nearest point on
    // the boundary, nudged slightly inward so contains() is guaranteed to
    // pass. Lets child regions hug the parent border: clicking just outside
    // the parent snaps the point onto the parent's edge instead of rejecting it.
    Vec2 closestPointInside(const Vec2& p) const
    {
        if (contains(p)) return p;

        const double eps = 0.05; // sub-pixel, invisible but beats the contains() jitter

        if (type == GeometryType::Rectangle)
        {
            return {
                std::clamp(p.x, rectMin.x + eps, rectMax.x - eps),
                std::clamp(p.y, rectMin.y + eps, rectMax.y - eps)
            };
        }

        Vec2 best;
        int bestEdge = closestBoundaryEdge(p, &best);
        if (bestEdge < 0) return p;

        std::vector<Vec2> pts = getPoints();
        int n = static_cast<int>(pts.size());

        // Nudge inward along the closest edge's interior-facing normal.
        const Vec2& ea = pts[bestEdge];
        const Vec2& eb = pts[(bestEdge + 1) % n];
        Vec2 d { eb.x - ea.x, eb.y - ea.y };
        double len = std::hypot(d.x, d.y);
        if (len <= 0.0) return best;

        Vec2 nrm = (signedArea() > 0.0)
            ? Vec2{ -d.y / len,  d.x / len }
            : Vec2{  d.y / len, -d.x / len };

        Vec2 q { best.x + nrm.x * eps, best.y + nrm.y * eps };
        if (!contains(q)) // concave corner edge case — try the other side
            q = { best.x - nrm.x * eps, best.y - nrm.y * eps };
        return q;
    }

    // True when the straight segment a→b stays inside the polygon.
    // Endpoints are assumed to already be inside (possibly hugging the border).
    bool segmentInside(const Vec2& a, const Vec2& b) const
    {
        std::vector<Vec2> pts = getPoints();
        int n = static_cast<int>(pts.size());
        if (n < 3) return true;

        auto cross = [](const Vec2& o, const Vec2& p, const Vec2& q) {
            return (p.x - o.x) * (q.y - o.y) - (p.y - o.y) * (q.x - o.x);
        };

        for (int i = 0; i < n; i++)
        {
            const Vec2& c = pts[i];
            const Vec2& d = pts[(i + 1) % n];

            double d1 = cross(c, d, a);
            double d2 = cross(c, d, b);
            double d3 = cross(a, b, c);
            double d4 = cross(a, b, d);

            // Strict signs only — endpoints touching the border don't count
            if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
                ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
                return false;
        }

        Vec2 mid { (a.x + b.x) * 0.5, (a.y + b.y) * 0.5 };
        return contains(mid);
    }

    // Vertices to insert between a and b so the path follows the boundary
    // instead of cutting across outside (walks the shorter way around the
    // ring). Each vertex is nudged inward; vertices nearly coinciding with
    // a or b are skipped to avoid degenerate edges.
    std::vector<Vec2> boundaryPathBetween(const Vec2& a, const Vec2& b) const
    {
        std::vector<Vec2> pts = getPoints();
        int n = static_cast<int>(pts.size());
        if (n < 3) return {};

        int ia = closestBoundaryEdge(a);
        int ib = closestBoundaryEdge(b);
        if (ia < 0 || ib < 0 || ia == ib) return {};

        // Vertex indices passed when walking forward / backward along the ring
        std::vector<int> fwd, bwd;
        for (int i = (ia + 1) % n, guard = 0; guard < n; i = (i + 1) % n, guard++)
        {
            fwd.push_back(i);
            if (i == ib) break;
        }
        for (int i = ia, guard = 0; guard < n; i = (i - 1 + n) % n, guard++)
        {
            bwd.push_back(i);
            if (i == (ib + 1) % n) break;
        }

        auto pathLen = [&](const std::vector<int>& idx) {
            double L = 0.0; Vec2 prev = a;
            for (int i : idx)
            {
                L += std::hypot(pts[i].x - prev.x, pts[i].y - prev.y);
                prev = pts[i];
            }
            return L + std::hypot(b.x - prev.x, b.y - prev.y);
        };

        const std::vector<int>& chosen = (pathLen(fwd) <= pathLen(bwd)) ? fwd : bwd;

        const double minDist = 0.2; // skip vertices that would create ~zero-length edges
        std::vector<Vec2> out;
        for (int i : chosen)
        {
            Vec2 v = nudgedVertex(i);
            if (std::hypot(v.x - a.x, v.y - a.y) < minDist) continue;
            if (std::hypot(v.x - b.x, v.y - b.y) < minDist) continue;
            out.push_back(v);
        }
        return out;
    }
};
