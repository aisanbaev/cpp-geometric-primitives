#include "convex_hull.hpp"
#include "geometry.hpp"
#include "intersections.hpp"
#include "queries.hpp"
#include "shape_utils.hpp"
#include "triangulation.hpp"
#include "visualization.hpp"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using namespace geometry;

namespace rng = std::ranges;
namespace views = std::ranges::views;

void PrintAllIntersections(const Shape &shape, std::span<const Shape> others) {
    std::println("\n=== Intersections ===");

    auto results = others | views::enumerate |
                   views::transform([&](const auto &pair) -> std::optional<std::tuple<size_t, Shape, Point2D>> {
                       const auto &[idx, other] = pair;

                       try {
                           auto intersection = intersections::GetIntersectPoint(shape, other);
                           if (intersection.has_value()) {
                               return std::make_tuple(idx, other, intersection.value());
                           }
                           return std::nullopt;
                       } catch (const std::logic_error &) {
                           return std::nullopt;
                       }
                   }) |
                   views::filter([](const auto &opt) { return opt.has_value(); }) |
                   views::transform([](const auto &opt) { return opt.value(); });

    rng::for_each(results, [&](const auto &tuple) {
        const auto &[other_idx, other_shape, point] = tuple;
        std::println("Intersection found at point {} between shape {} and shape {}", point, shape, other_shape);
    });

    if (rng::empty(results)) {
        std::println("No intersections found between supported shape types.");
    }
}

void PrintDistancesFromPointToShapes(Point2D p, std::span<const Shape> shapes) {
    std::println("\n=== Distance from Point Test ===");

    auto selected_shapes = shapes | views::take(5) | views::enumerate;

    rng::for_each(selected_shapes, [&](const auto &pair) {
        const auto &[idx, shape] = pair;
        double distance = queries::DistanceToPoint(shape, p);
        std::println("Distance from {} to shape {}: {} = {:.2f}", p, idx, shape, distance);
    });
}

void PerformShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Analysis ===");

    if (shapes.empty()) {
        std::println("No shapes to analyze.");
        return;
    }

    // 1. Находим все пересечения между фигурами используя метод Bounding Box
    auto collisions = utils::FindAllCollisions(shapes);

    std::println("Bounding Box intersections found: {}", collisions.size());
    rng::for_each(collisions, [](const auto &pair) {
        const auto &[shape1, shape2] = pair;
        std::println("  Collision between {} and {}", shape1, shape2);
    });

    // 2. Самая высокая фигура
    if (auto idx = utils::FindHighestShape(shapes)) {
        std::println("\nHighest shape (index {}): {}", *idx, shapes[*idx]);
        std::println("  Height: {:.2f}", queries::GetHeight(shapes[*idx]));
    } else {
        std::println("\nCould not determine the highest shape.");
    }

    // 3. Выводим расстояние между любыми двумя фигурами, которые поддерживают данную функциональность
    std::println("\nDistances between supported shapes:");

    auto indices = views::iota(0uz, shapes.size());
    auto pairs = views::cartesian_product(indices, indices) | views::filter([](const auto &p) {
                     const auto &[i, j] = p;
                     return i < j;
                 }) |
                 views::transform([&](const auto &p) {
                     const auto &[i, j] = p;
                     return std::make_pair(p, geometry::queries::DistanceBetweenShapes(shapes[i], shapes[j]));
                 }) |
                 views::filter([](const auto &p) { return p.second.has_value(); }) |
                 views::transform([](const auto &p) {
                     const auto &[indices, dist] = p;
                     return std::make_tuple(std::get<0>(indices), std::get<1>(indices), dist.value());
                 }) |
                 views::take(5);

    bool has_supported_pairs = false;
    rng::for_each(pairs, [&](const auto &tuple) {
        const auto &[i, j, distance] = tuple;
        std::println("  Distance between {} and {}: {:.2f}", shapes[i], shapes[j], distance);
        has_supported_pairs = true;
    });

    if (!has_supported_pairs) {
        std::println("  No supported shape pairs for distance calculation");
    }
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    using namespace queries;
    std::println("\n=== Shape Extra Analysis ===");

    // 1. Выводим 3 любые фигуры, которые находятся выше 50.0
    auto high_shapes = shapes | views::enumerate | views::filter([](const auto &pair) {
                           const auto &[idx, shape] = pair;
                           return GetHeight(shape) > 50.0;
                       }) |
                       views::take(3);

    std::println("Shapes with height > 50.0:");
    rng::for_each(high_shapes, [](const auto &pair) {
        const auto &[idx, shape] = pair;
        std::println("  Shape {}: height {:.2f}", idx, GetHeight(shape));
    });

    if (high_shapes.empty()) {
        std::println("  No shapes found above height 50.0");
    }

    // 2. Выводим фигуры с наименьшей и наибольшей высотами
    if (shapes.empty()) {
        std::println("No shapes available for height analysis");
        return;
    }

    auto max_height_it =
        rng::max_element(shapes, [](const Shape &a, const Shape &b) { return GetHeight(a) < GetHeight(b); });

    auto min_height_it =
        rng::min_element(shapes, [](const Shape &a, const Shape &b) { return GetHeight(a) < GetHeight(b); });

    size_t max_idx = std::distance(shapes.begin(), max_height_it);
    size_t min_idx = std::distance(shapes.begin(), min_height_it);

    std::println("Shape with maximum height: {} (height: {:.2f})", shapes[max_idx], GetHeight(shapes[max_idx]));
    std::println("Shape with minimum height: {} (height: {:.2f})", shapes[min_idx], GetHeight(shapes[min_idx]));
}

int main() {
    std::vector<Shape> shapes = utils::ParseShapes("circle -3 0 2.0; "
                                                   "line -4 0 12 0; "
                                                   "polygon 10 2 3.5 6; "
                                                   "triangle -3 1 2 1 -2 4; "
                                                   "rectangle -5 -4 3 2; "
                                                   "circle 5 5 6.5");
    std::println("Parsed {} shapes", shapes.size());

    // Выводим индекс каждой фигуры и её высоту
    std::println("\n=== Shapes and their heights ===");
    rng::for_each(shapes | views::enumerate, [](const auto &pair) {
        const auto &[idx, shape] = pair;
        std::println("Shape {}: height {:.2f}", idx, queries::GetHeight(shape));
    });

    if (!shapes.empty()) {
        PrintAllIntersections(shapes[0], shapes);
        PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, shapes);
        PerformShapeAnalysis(shapes);
        PerformExtraShapeAnalysis(shapes);
    }

    //
    // Рисуем все фигуры
    //
    std::println("\n=== Drawing all shapes ===");
    geometry::visualization::Draw(shapes, "output_1_shapes.png");

    //
    // Формируем список из вершин всех фигур
    //
    std::vector<Point2D> points;
    for (const auto &shape : shapes) {
        std::visit(
            [&](const auto &geom) {
                if constexpr (requires { geom.Vertices(); }) {
                    auto vertices = geom.Vertices();
                    points.insert(points.end(), vertices.begin(), vertices.end());
                }
            },
            shape);
    }

    std::println("\nCollected {} points from all shapes", points.size());

    //
    // Находим выпуклую оболочку, добавляем её в shapes и рисуем итоговый вариант
    //
    if (points.size() >= 3) {
        auto hull_result = convex_hull::GrahamScan(points);
        if (hull_result.has_value()) {
            auto hull_points = hull_result.value();
            if (hull_points.size() >= 3) {
                shapes.emplace_back(Polygon{hull_points});
                std::println("Added convex hull with {} points", hull_points.size());
            }
        } else {
            std::println("Failed to compute convex hull: {}", hull_result.error());
        }
    }
    geometry::visualization::Draw(shapes, "output_2_with_hull.png");

    //
    // Триангуляция Делоне
    //
    {
        std::vector<Point2D> points = {{0, 0}, {10, 0}, {5, 8}, {15, 5}, {2, 12}};
        auto triangulation_result = triangulation::DelaunayTriangulation(points);

        if (triangulation_result.has_value()) {
            auto triangles = triangulation_result.value();
            std::println("\nDelaunay triangulation created {} triangles", triangles.size());

            // Конвертируем треугольники в фигуры для отрисовки
            std::vector<Shape> triangle_shapes;
            for (const auto &triangle : triangles) {
                triangle_shapes.emplace_back(Triangle{triangle.a, triangle.b, triangle.c});
            }
            geometry::visualization::Draw(triangle_shapes, "output_3_triangulation.png");

        } else {
            std::println("Failed to compute Delaunay triangulation: {}", triangulation_result.error());
        }
    }

    return 0;
}