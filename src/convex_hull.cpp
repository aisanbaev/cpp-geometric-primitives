#include "convex_hull.hpp"
#include <algorithm>
#include <stdexcept>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2) {
    auto new_p1 = p1 - middle;
    auto new_p2 = p2 - middle;
    return new_p1.Cross(new_p2);
}

std::expected<std::vector<Point2D>, std::string> GrahamScan(std::span<Point2D> points) noexcept {
    if (points.size() < 3) {
        return std::unexpected("At least three points are required for convex hull.");
    }

    // Находим минимальную точку
    auto min_it = std::min_element(points.begin(), points.end(), [](const Point2D &a, const Point2D &b) {
        return a.y < b.y || (a.y == b.y && a.x < b.x);
    });

    // Перемещаем её в начало
    std::iter_swap(points.begin(), min_it);
    Point2D pivot = points[0];

    std::sort(points.begin() + 1, points.end(), [&pivot](const Point2D &a, const Point2D &b) {
        static const auto precision = 1e-10;
        double cross = (a - pivot).Cross(b - pivot);
        if (std::abs(cross) < precision) {
            return pivot.DistanceTo(a) < pivot.DistanceTo(b);
        }
        return cross > 0;
    });

    StackForGrahamScan hull;
    for (const auto &new_p : points) {
        while (hull.Size() > 1 && CrossProduct(hull.NextToTop(), hull.Top(), new_p) > 0.0) {
            hull.Pop();
        }
        hull.Push(new_p);
    }

    return std::vector{hull.Extract()};
}

}  // namespace geometry::convex_hull