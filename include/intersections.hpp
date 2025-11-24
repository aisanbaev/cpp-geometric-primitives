#pragma once
#include "geometry.hpp"
#include <cmath>
#include <optional>
#include <stdexcept>

namespace geometry::intersections {

class IntersectionVisitor {
    static constexpr double EPS = 1e-9;

public:
    [[nodiscard]] std::optional<Point2D> operator()(const Line &l1, const Line &l2) const {
        const Point2D r = l1.end - l1.start;
        const Point2D s = l2.end - l2.start;
        const Point2D w = l2.start - l1.start;

        const double r_cross_s = r.Cross(s);

        if (std::abs(r_cross_s) < EPS) {
            // Параллельные или совпадающие линии
            return std::nullopt;
        }

        const double t = w.Cross(s) / r_cross_s;
        const double u = w.Cross(r) / r_cross_s;

        // Проверяем что пересечение в пределах отрезков
        if (t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0) {
            return l1.start + r * t;
        }

        return std::nullopt;
    }

    [[nodiscard]] std::optional<Point2D> operator()(const Line &line, const Circle &circle) const {
        return intersectLineCircle(line, circle);
    }

    [[nodiscard]] std::optional<Point2D> operator()(const Circle &circle, const Line &line) const {
        return intersectLineCircle(line, circle);
    }

    [[nodiscard]] std::optional<Point2D> operator()(const Circle &c1, const Circle &c2) const {
        const Point2D d = c2.center_p - c1.center_p;
        const double dist = d.Length();

        if (dist < EPS) {
            return std::nullopt;  // Совпадающие центры
        }

        // Проверка на отсутствие пересечения
        if (dist > c1.radius + c2.radius + EPS)
            return std::nullopt;
        if (dist + std::min(c1.radius, c2.radius) < std::max(c1.radius, c2.radius) - EPS) {
            return std::nullopt;  // Одна окружность внутри другой
        }

        const double a = (c1.radius * c1.radius - c2.radius * c2.radius + dist * dist) / (2 * dist);
        double h_sq = c1.radius * c1.radius - a * a;

        if (h_sq < -EPS)
            return std::nullopt;
        if (h_sq < 0)
            h_sq = 0;

        const double h = std::sqrt(h_sq);
        const Point2D ex = d / dist;
        const Point2D ey{-ex.y, ex.x};

        // Возвращаем одну точку пересечения (ближайшую к первой окружности)
        return c1.center_p + ex * a + ey * h;
    }

    template <typename T, typename U>
    [[nodiscard]] std::optional<Point2D> operator()(const T &, const U &) const {
        throw std::logic_error("Intersection not supported for these types");
    }

private:
    [[nodiscard]] std::optional<Point2D> intersectLineCircle(const Line &line, const Circle &circle) const {
        const Point2D d = line.end - line.start;
        const Point2D f = line.start - circle.center_p;

        const double a = d.Dot(d);

        // Случай: отрезок — это точка
        if (a < EPS) {
            double dist_sq = f.Dot(f);
            double radius_sq = circle.radius * circle.radius;
            if (std::abs(dist_sq - radius_sq) < EPS) {
                return line.start;
            }
            return std::nullopt;
        }

        const double b = 2.0 * f.Dot(d);
        const double c = f.Dot(f) - circle.radius * circle.radius;

        const double discriminant = b * b - 4.0 * a * c;

        if (discriminant < -EPS)
            return std::nullopt;

        if (std::abs(discriminant) <= EPS) {
            // Касание
            const double t = -b / (2.0 * a);
            if (t >= 0.0 && t <= 1.0) {
                return line.start + d * t;
            }
        } else {
            // Два пересечения
            const double sqrt_d = std::sqrt(discriminant);
            const double t1 = (-b - sqrt_d) / (2.0 * a);
            const double t2 = (-b + sqrt_d) / (2.0 * a);

            // Возвращаем ближайшее пересечение от начала отрезка
            if (t1 >= 0.0 && t1 <= 1.0)
                return line.start + d * t1;
            if (t2 >= 0.0 && t2 <= 1.0)
                return line.start + d * t2;
        }

        return std::nullopt;
    }
};

[[nodiscard]] inline std::optional<Point2D> GetIntersectPoint(const Shape &shape1, const Shape &shape2) {
    return std::visit(IntersectionVisitor{}, shape1, shape2);
}

}  // namespace geometry::intersections