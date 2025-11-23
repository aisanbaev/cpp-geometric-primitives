#include "queries.hpp"
#include <gtest/gtest.h>

using namespace geometry;
using namespace geometry::queries;

TEST(QueriesTest, DistanceToPoint_Line) {
    Line line{{0, 0}, {2, 0}};
    Point2D p{1, 1};
    EXPECT_DOUBLE_EQ(DistanceToPoint(line, p), 1.0);
}

TEST(QueriesTest, DistanceToPoint_Circle) {
    Circle circle{{0, 0}, 1.0};
    Point2D p{2, 0};
    EXPECT_DOUBLE_EQ(DistanceToPoint(circle, p), 1.0);  // 2 - 1 = 1

    Point2D inside{0.5, 0};
    EXPECT_DOUBLE_EQ(DistanceToPoint(circle, inside), 0.0);  // внутри → 0
}

TEST(QueriesTest, DistanceToPoint_Triangle) {
    Triangle tri{{0, 0}, {2, 0}, {1, 2}};
    Point2D p{1, 3};  // над вершиной
    double dist = DistanceToPoint(tri, p);
    EXPECT_NEAR(dist, 1.0, 1e-6);  // расстояние до вершины (1,2)
}

TEST(QueriesTest, DistanceToPoint_Rectangle) {
    Rectangle rect{{1, 1}, 2, 2};

    Point2D p_right{4, 2};
    EXPECT_DOUBLE_EQ(DistanceToPoint(rect, p_right), 1.0);

    Point2D p_top{2, 4};
    EXPECT_DOUBLE_EQ(DistanceToPoint(rect, p_top), 1.0);
}

TEST(QueriesTest, DistanceToPoint_Polygon_ActualBehavior) {
    std::vector<Point2D> points{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    Polygon poly{points};

    Point2D p_right{3, 1};
    EXPECT_NEAR(DistanceToPoint(poly, p_right), std::sqrt(2.0), 1e-9);

    Point2D p_center{1, 1};
    EXPECT_NEAR(DistanceToPoint(poly, p_center), std::sqrt(2.0), 1e-9);
}

TEST(QueriesTest, GetBoundBox_Line) {
    Line line{{1, 2}, {3, 4}};
    BoundingBox bb = GetBoundBox(line);
    EXPECT_EQ(bb.min_x, 1);
    EXPECT_EQ(bb.min_y, 2);
    EXPECT_EQ(bb.max_x, 3);
    EXPECT_EQ(bb.max_y, 4);
}

TEST(QueriesTest, GetBoundBox_Circle) {
    Circle circle{{0, 0}, 2.0};
    BoundingBox bb = GetBoundBox(circle);
    EXPECT_EQ(bb.min_x, -2);
    EXPECT_EQ(bb.min_y, -2);
    EXPECT_EQ(bb.max_x, 2);
    EXPECT_EQ(bb.max_y, 2);
}

TEST(QueriesTest, GetHeight_Rectangle) {
    Rectangle rect{{0, 0}, 3, 4};
    EXPECT_DOUBLE_EQ(GetHeight(rect), 4);  // bottom_left.y + height = 0 + 4
}

TEST(QueriesTest, BoundingBoxesOverlap_Yes) {
    Circle c1{{0, 0}, 1};
    Circle c2{{1, 0}, 1};
    EXPECT_TRUE(BoundingBoxesOverlap(c1, c2));
}

TEST(QueriesTest, BoundingBoxesOverlap_No) {
    Circle c1{{0, 0}, 1};
    Circle c2{{3, 0}, 1};
    EXPECT_FALSE(BoundingBoxesOverlap(c1, c2));
}

TEST(QueriesTest, DistanceBetweenShapes_CircleCircle) {
    Circle c1{{0, 0}, 1};
    Circle c2{{3, 0}, 1};
    auto dist = DistanceBetweenShapes(c1, c2);
    ASSERT_TRUE(dist.has_value());
    EXPECT_DOUBLE_EQ(*dist, 1.0);  // 3 - 1 - 1 = 1
}

TEST(QueriesTest, DistanceBetweenShapes_LineLine) {
    Line l1{{0, 0}, {0, 1}};
    Line l2{{1, 0}, {1, 1}};
    auto dist = DistanceBetweenShapes(l1, l2);
    ASSERT_TRUE(dist.has_value());
    EXPECT_DOUBLE_EQ(*dist, 1.0);  // расстояние между параллельными отрезками
}

TEST(QueriesTest, DistanceBetweenShapes_Unsupported) {
    Triangle t{{0, 0}, {1, 0}, {0, 1}};
    Circle c{{2, 2}, 1};
    auto dist = DistanceBetweenShapes(t, c);
    EXPECT_FALSE(dist.has_value());  // не поддерживается
}

// ========================================================
// constexpr тесты
// ========================================================

namespace {
constexpr Shape make_circle() { return Circle{{1.0, 2.0}, 3.0}; }
constexpr Shape make_line() { return Line{{0.0, 0.0}, {4.0, 5.0}}; }
constexpr Shape make_rect() { return Rectangle{{-1.0, -1.0}, 2.0, 3.0}; }
}  // namespace

TEST(QueriesConstexprTest, GetBoundBox_Circle) {
    constexpr Shape s = make_circle();
    constexpr BoundingBox bb = GetBoundBox(s);
    static_assert(bb.min_x == -2.0);
    static_assert(bb.min_y == -1.0);
    static_assert(bb.max_x == 4.0);
    static_assert(bb.max_y == 5.0);
}

TEST(QueriesConstexprTest, GetBoundBox_Line) {
    constexpr Shape s = make_line();
    constexpr BoundingBox bb = GetBoundBox(s);
    static_assert(bb.min_x == 0.0);
    static_assert(bb.min_y == 0.0);
    static_assert(bb.max_x == 4.0);
    static_assert(bb.max_y == 5.0);
}

TEST(QueriesConstexprTest, GetHeight_Rectangle) {
    constexpr Shape s = make_rect();
    constexpr double h = GetHeight(s);
    static_assert(h == 2.0);  // bottom_left.y + height = -1 + 3 = 2
}

TEST(QueriesConstexprTest, BoundingBoxesOverlap) {
    constexpr Shape s1 = Circle{{0.0, 0.0}, 1.0};
    constexpr Shape s2 = Circle{{0.5, 0.0}, 1.0};
    constexpr bool overlap = BoundingBoxesOverlap(s1, s2);
    static_assert(overlap == true);
}

// Попытка использовать DistanceToPoint в constexpr — не скомпилируется
// static_assert(DistanceToPoint(Circle{{0, 0}, 1}, {2, 0}) == 1.0);  // ОШИБКА
