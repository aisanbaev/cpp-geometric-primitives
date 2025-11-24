#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <expected>
#include <format>
#include <ranges>
#include <set>
#include <vector>

namespace geometry::triangulation {

static constexpr double EPS = 1e-10;

struct DelaunayTriangle {
    Point2D a, b, c;

    constexpr DelaunayTriangle(Point2D a, Point2D b, Point2D c) : a(a), b(b), c(c) {}

    [[nodiscard]] bool ContainsPoint(const Point2D &p) const {
        Point2D center = Circumcenter();
        double radius = Circumradius();
        return center.DistanceTo(p) <= radius + EPS;
    }

    [[nodiscard]] Point2D Circumcenter() const {
        double d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
        if (std::abs(d) < EPS) {
            return {(a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3};
        }

        double ux = ((a.x * a.x + a.y * a.y) * (b.y - c.y) + (b.x * b.x + b.y * b.y) * (c.y - a.y) +
                     (c.x * c.x + c.y * c.y) * (a.y - b.y)) /
                    d;

        double uy = ((a.x * a.x + a.y * a.y) * (c.x - b.x) + (b.x * b.x + b.y * b.y) * (a.x - c.x) +
                     (c.x * c.x + c.y * c.y) * (b.x - a.x)) /
                    d;

        return {ux, uy};
    }

    [[nodiscard]] double Circumradius() const { return Circumcenter().DistanceTo(a); }

    [[nodiscard]] bool SharesEdge(const DelaunayTriangle &other) const {
        auto this_points = {a, b, c};
        auto other_points = {other.a, other.b, other.c};

        auto shared_count = std::ranges::count_if(this_points, [&](const Point2D &p1) {
            return std::ranges::any_of(other_points, [&](const Point2D &p2) { return p1.DistanceTo(p2) < EPS; });
        });

        return shared_count == 2;
    }

    std::vector<Point2D> vertices() const { return {a, b, c}; }
};

struct Edge {
    Point2D p1, p2;

    Edge(Point2D p1, Point2D p2) : p1(p1), p2(p2) {
        if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y)) {
            std::swap(this->p1, this->p2);
        }
    }

    [[nodiscard]] constexpr bool operator<(const Edge &other) const {
        if (std::abs(p1.x - other.p1.x) > EPS)
            return p1.x < other.p1.x;
        if (std::abs(p1.y - other.p1.y) > EPS)
            return p1.y < other.p1.y;
        if (std::abs(p2.x - other.p2.x) > EPS)
            return p2.x < other.p2.x;
        return p2.y < other.p2.y;
    }

    [[nodiscard]] constexpr bool operator==(const Edge &other) const {
        return p1.DistanceTo(other.p1) < EPS && p2.DistanceTo(other.p2) < EPS;
    }
};

[[nodiscard]] inline std::expected<std::vector<DelaunayTriangle>, std::string>
DelaunayTriangulation(std::span<const Point2D> points) noexcept {
    if (points.size() < 3) {
        return std::unexpected("At least three points are required for triangulation.");
    }

    auto x_proj = std::views::transform(points, [](const Point2D &p) { return p.x; });
    auto y_proj = std::views::transform(points, [](const Point2D &p) { return p.y; });

    auto [min_x, max_x] = std::ranges::minmax(x_proj);
    auto [min_y, max_y] = std::ranges::minmax(y_proj);

    double dx = max_x - min_x;
    double dy = max_y - min_y;
    double dmax = std::max(dx, dy);
    Point2D center = {(min_x + max_x) / 2, (min_y + max_y) / 2};

    Point2D super1 = {center.x - 20 * dmax, center.y - dmax};
    Point2D super2 = {center.x, center.y + 20 * dmax};
    Point2D super3 = {center.x + 20 * dmax, center.y - dmax};

    std::vector<DelaunayTriangle> triangles;
    triangles.emplace_back(super1, super2, super3);

    for (const Point2D &point : points) {
        std::vector<DelaunayTriangle> bad_triangles;
        std::set<Edge> polygon;

        for (const auto &triangle : triangles) {
            if (triangle.ContainsPoint(point)) {
                bad_triangles.push_back(triangle);

                for (const Edge &edge :
                     {Edge{triangle.a, triangle.b}, Edge{triangle.b, triangle.c}, Edge{triangle.c, triangle.a}}) {
                    if (!polygon.erase(edge)) {
                        polygon.insert(edge);
                    }
                }
            }
        }

        std::erase_if(triangles, [&bad_triangles](const DelaunayTriangle &t) {
            return std::ranges::any_of(bad_triangles, [&t](const DelaunayTriangle &bad) {
                return t.a.DistanceTo(bad.a) < EPS && t.b.DistanceTo(bad.b) < EPS && t.c.DistanceTo(bad.c) < EPS;
            });
        });

        for (const Edge &edge : polygon) {
            triangles.emplace_back(edge.p1, edge.p2, point);
        }
    }

    auto is_super_triangle_vertex = [&](const Point2D &p) {
        return p.DistanceTo(super1) < EPS || p.DistanceTo(super2) < EPS || p.DistanceTo(super3) < EPS;
    };

    std::erase_if(triangles, [&](const DelaunayTriangle &t) {
        return std::ranges::any_of(t.vertices(), is_super_triangle_vertex);
    });

    return triangles;
}

}  // namespace geometry::triangulation

template <>
struct std::formatter<geometry::triangulation::DelaunayTriangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::triangulation::DelaunayTriangle &t, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "DelaunayTriangle({}, {}, {})", t.a, t.b, t.c);
    }
};