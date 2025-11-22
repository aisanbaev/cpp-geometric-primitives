#include "intersections.hpp"
#include <gtest/gtest.h>

using namespace geometry;
using namespace geometry::intersections;

TEST(IntersectionTest, LineLineIntersection) {
    // Пересекающиеся линии
    Line l1{Point2D{0.0, 0.0}, Point2D{2.0, 2.0}};
    Line l2{Point2D{0.0, 2.0}, Point2D{2.0, 0.0}};

    auto result = GetIntersectPoint(l1, l2);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 1.0, 1e-9);
    EXPECT_NEAR(result->y, 1.0, 1e-9);

    // Параллельные линии
    Line l3{Point2D{0.0, 0.0}, Point2D{1.0, 0.0}};
    Line l4{Point2D{0.0, 1.0}, Point2D{1.0, 1.0}};
    EXPECT_FALSE(GetIntersectPoint(l3, l4).has_value());

    // Совпадающие линии
    Line l5{Point2D{0.0, 0.0}, Point2D{1.0, 1.0}};
    Line l6{Point2D{0.5, 0.5}, Point2D{2.0, 2.0}};
    EXPECT_FALSE(GetIntersectPoint(l5, l6).has_value());
}

TEST(IntersectionTest, LineLineNoIntersection) {
    Line l1{Point2D{0.0, 0.0}, Point2D{1.0, 1.0}};
    Line l2{Point2D{2.0, 0.0}, Point2D{0.0, 3.0}};
    EXPECT_FALSE(GetIntersectPoint(l1, l2).has_value());

    Line l3{Point2D{0.0, 0.0}, Point2D{0.0, 1.0}};
    Line l4{Point2D{1.0, 1.0}, Point2D{2.0, 1.0}};
    EXPECT_FALSE(GetIntersectPoint(l3, l4).has_value());
}

TEST(IntersectionTest, LineCircleIntersection) {
    // Касание окружности
    Circle circle{Point2D{0.0, 0.0}, 1.0};
    Line tangent{Point2D{1.0, -1.0}, Point2D{1.0, 1.0}};  // Касается в (1,0)

    auto result = GetIntersectPoint(tangent, circle);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 1.0, 1e-9);
    EXPECT_NEAR(result->y, 0.0, 1e-9);

    // Пересечение в двух точках (возвращаем первую)
    Line secant{Point2D{-2.0, 0.0}, Point2D{2.0, 0.0}};  // Пересекает в (-1,0) и (1,0)
    result = GetIntersectPoint(secant, circle);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, -1.0, 1e-9);
    EXPECT_NEAR(result->y, 0.0, 1e-9);

    // Без пересечения
    Line outside{Point2D{2.0, 2.0}, Point2D{3.0, 3.0}};
    EXPECT_FALSE(GetIntersectPoint(outside, circle).has_value());
}

TEST(IntersectionTest, CircleLineIntersection) {
    // Проверка коммутативности
    Circle circle{Point2D{0.0, 0.0}, 2.0};
    Line line{Point2D{-3.0, 0.0}, Point2D{3.0, 0.0}};

    auto result1 = GetIntersectPoint(line, circle);
    auto result2 = GetIntersectPoint(circle, line);

    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_NEAR(result1->x, result2->x, 1e-9);
    EXPECT_NEAR(result1->y, result2->y, 1e-9);
    EXPECT_NEAR(result1->x, -2.0, 1e-9);
}

TEST(IntersectionTest, CircleCircleIntersection) {
    // Пересекающиеся окружности
    Circle c1{Point2D{0.0, 0.0}, 2.0};
    Circle c2{Point2D{3.0, 0.0}, 2.0};

    auto result = GetIntersectPoint(c1, c2);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 1.5, 1e-9);
    EXPECT_NEAR(result->y, std::sqrt(4.0 - 2.25), 1e-9);  // sqrt(2^2 - 1.5^2)

    // Касающиеся окружности
    Circle c3{Point2D{0.0, 0.0}, 1.0};
    Circle c4{Point2D{2.0, 0.0}, 1.0};
    result = GetIntersectPoint(c3, c4);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 1.0, 1e-9);
    EXPECT_NEAR(result->y, 0.0, 1e-9);

    // Непересекающиеся окружности
    Circle c5{Point2D{0.0, 0.0}, 1.0};
    Circle c6{Point2D{3.0, 0.0}, 1.0};
    EXPECT_FALSE(GetIntersectPoint(c5, c6).has_value());

    // Одна окружность внутри другой
    Circle c7{Point2D{0.0, 0.0}, 1.0};
    Circle c8{Point2D{0.0, 0.0}, 2.0};
    EXPECT_FALSE(GetIntersectPoint(c7, c8).has_value());
}

TEST(IntersectionTest, UnsupportedIntersections) {
    Line line{Point2D{0.0, 0.0}, Point2D{1.0, 1.0}};
    Triangle tri{Point2D{0.0, 0.0}, Point2D{1.0, 0.0}, Point2D{0.0, 1.0}};
    Rectangle rect{Point2D{0.0, 0.0}, 1.0, 1.0};

    EXPECT_THROW(GetIntersectPoint(line, tri), std::logic_error);
    EXPECT_THROW(GetIntersectPoint(tri, line), std::logic_error);
    EXPECT_THROW(GetIntersectPoint(rect, tri), std::logic_error);
    EXPECT_THROW(GetIntersectPoint(tri, tri), std::logic_error);
}

TEST(IntersectionTest, EdgeCases) {
    // Нулевая длина линии
    Line zero_line{Point2D{1.0, 0.0}, Point2D{1.0, 0.0}};
    Circle circle{Point2D{0.0, 0.0}, 1.0};

    auto result = GetIntersectPoint(zero_line, circle);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 1.0, 1e-9);
    EXPECT_NEAR(result->y, 0.0, 1e-9);

    // Точка вне окружности
    Line point_outside{Point2D{2.0, 2.0}, Point2D{2.0, 2.0}};
    EXPECT_FALSE(GetIntersectPoint(point_outside, circle).has_value());

    // Очень маленькие окружности
    Circle small1{Point2D{0.0, 0.0}, 1e-6};
    Circle small2{Point2D{1e-6, 0.0}, 1e-6};
    result = GetIntersectPoint(small1, small2);
    EXPECT_TRUE(result.has_value());
}

TEST(IntersectionTest, PrecisionTesting) {
    Circle c1{Point2D{0.0, 0.0}, 1000.0};
    Circle c2{Point2D{2000.0, 0.0}, 1000.0};

    auto result = GetIntersectPoint(c1, c2);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 1000.0, 1e-9);
    EXPECT_NEAR(result->y, 0.0, 1e-9);

    Line big_line{Point2D{1e6, 1e6}, Point2D{2e6, 2e6}};
    Circle big_circle{Point2D{1.5e6, 1.5e6}, 1e5};

    result = GetIntersectPoint(big_line, big_circle);
    EXPECT_TRUE(result.has_value());  // Линия проходит через центр
}

TEST(IntersectionTest, CommutativeProperty) {
    // Проверка коммутативности для всех поддерживаемых пар
    Line line{Point2D{0.0, 0.0}, Point2D{1.0, 1.0}};
    Circle circle{Point2D{0.0, 0.0}, 2.0};
    Circle circle2{Point2D{1.0, 0.0}, 1.0};

    auto line_circle = GetIntersectPoint(line, circle);
    auto circle_line = GetIntersectPoint(circle, line);

    if (line_circle.has_value()) {
        ASSERT_TRUE(circle_line.has_value());
        EXPECT_NEAR(line_circle->x, circle_line->x, 1e-9);
        EXPECT_NEAR(line_circle->y, circle_line->y, 1e-9);
    } else {
        EXPECT_FALSE(circle_line.has_value());
    }

    auto circle_circle2 = GetIntersectPoint(circle, circle2);
    auto circle2_circle = GetIntersectPoint(circle2, circle);

    if (circle_circle2.has_value()) {
        ASSERT_TRUE(circle2_circle.has_value());
        EXPECT_NEAR(circle_circle2->x, circle2_circle->x, 1e-9);
        EXPECT_NEAR(circle_circle2->y, circle2_circle->y, 1e-9);
    } else {
        EXPECT_FALSE(circle2_circle.has_value());
    }
}