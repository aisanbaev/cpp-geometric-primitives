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
                   views::transform([&](const auto &pair) -> std::optional<std::pair<size_t, Point2D>> {
                       const auto &[idx, other] = pair;

                       try {
                           auto intersection = intersections::GetIntersectPoint(shape, other);
                           if (intersection.has_value()) {
                               return std::make_pair(idx, intersection.value());
                           }
                           // Если пересечения нет, но типы поддерживаются - возвращаем nullopt
                           return std::nullopt;
                       } catch (const std::logic_error &) {
                           // Неподдерживаемая комбинация типов - пропускаем
                           return std::nullopt;
                       }
                   }) |
                   views::filter([](const auto &opt) { return opt.has_value(); }) |
                   views::transform([](const auto &opt) { return opt.value(); });

    rng::for_each(results, [](const auto &pair) {
        const auto &[other_idx, point] = pair;
        std::println("Intersection found at point {} between shape 0 and shape {}", point, other_idx);
    });

    // Если нет пересечений
    if (rng::empty(results)) {
        std::println("No intersections found between supported shape types.");
    }
}

void PrintDistancesFromPointToShapes(Point2D p, std::span<const Shape> shapes) {
    std::println("\n=== Distance from Point Test ===");

    // Выбираем любые 5 фигур из списка
    auto selected_shapes = shapes | views::take(5);

    rng::for_each(selected_shapes, [&](const Shape &shape) {
        double distance = queries::DistanceToPoint(shape, p);
        std::println("Distance from point {} to shape is {:.2f}", p, distance);
    });
}

void PerformShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Analysis ===");

    // 1. Находим все пересечения между фигурами используя метод Bounding Box
    auto collisions = utils::FindAllCollisions(shapes);
    std::println("Found {} collisions using Bounding Box method", collisions.size());

    // 2. Находим самую высокую фигуру
    auto highest_idx = utils::FindHighestShape(shapes);
    if (highest_idx.has_value()) {
        std::println("Highest shape is at index {} with height {:.2f}", highest_idx.value(),
                     queries::GetHeight(shapes[highest_idx.value()]));
    }

    // 3. Выводим расстояние между любыми двумя фигурами, которые поддерживают данную функциональность
    if (shapes.size() >= 2) {
        auto distances = views::iota(0uz, std::min(shapes.size(), size_t(3))) | views::transform([&](size_t i) {
                             return std::make_pair(i, queries::DistanceBetweenShapes(shapes[0], shapes[i]));
                         }) |
                         views::filter([](const auto &pair) { return pair.second.has_value(); });

        rng::for_each(distances, [&](const auto &pair) {
            const auto &[idx, distance] = pair;
            std::println("Distance between shape 0 and shape {} is {:.2f}", idx, distance.value());
        });
    }
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Extra Analysis ===");

    // 1. Выводим 3 любые фигуры, которые находятся выше 50.0
    auto high_shapes = shapes | views::enumerate | views::filter([](const auto &pair) {
                           const auto &[idx, shape] = pair;
                           return queries::GetHeight(shape) > 50.0;
                       }) |
                       views::take(3);

    std::println("Shapes with height > 50.0:");
    rng::for_each(high_shapes, [](const auto &pair) {
        const auto &[idx, shape] = pair;
        std::println("  Shape {}: height {:.2f}", idx, queries::GetHeight(shape));
    });

    // 2. Выводим фигуры с наименьшей и с наибольшей высотами
    if (!shapes.empty()) {
        auto heights_view = shapes | views::transform([](const Shape &s) { return queries::GetHeight(s); });

        auto min_height = rng::min(heights_view);
        auto max_height = rng::max(heights_view);

        auto min_shape =
            rng::find_if(shapes, [&](const Shape &s) { return std::abs(queries::GetHeight(s) - min_height) < 1e-9; });

        auto max_shape =
            rng::find_if(shapes, [&](const Shape &s) { return std::abs(queries::GetHeight(s) - max_height) < 1e-9; });

        if (min_shape != shapes.end()) {
            std::println("Shape with minimum height {:.2f}", min_height);
        }
        if (max_shape != shapes.end()) {
            std::println("Shape with maximum height {:.2f}", max_height);
        }
    }
}

int main() {
    // std::vector<Shape> shapes = utils::ParseShapes("circle 0 0 1.5; line 1 2 3 4; polygon 0 0 2 5; triangle 0 0 1 0 "
    //                                                "0.5 1; polygon 0 0 1 2; badshape; circle 0 0 -1");
    std::vector<Shape> shapes = utils::ParseShapes("circle -3 0 2.0; "
                                                   "line -4 0 12 0; "
                                                   "polygon 10 2 1.5 6; "
                                                   "triangle -3 1 2 1 -2 4; "
                                                   "rectangle -5 -4 3 2");
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

    std::println("Collected {} points from all shapes", points.size());

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

        // Выполняем алгоритм триангуляции Делоне
        auto triangulation_result = triangulation::DelaunayTriangulation(points);

        if (triangulation_result.has_value()) {
            auto triangles = triangulation_result.value();
            std::println("Delaunay triangulation created {} triangles", triangles.size());

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