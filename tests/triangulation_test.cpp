#include "triangulation.hpp"
#include <gtest/gtest.h>

using namespace geometry;
using namespace geometry::triangulation;

TEST(TriangulationTest, DelaunayTriangulation_Success) {
    std::vector<Point2D> points = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};

    auto result = DelaunayTriangulation(points);
    ASSERT_TRUE(result.has_value());
    EXPECT_GE(result->size(), 2);  // Минимум 2 треугольника для 4 точек
}

TEST(TriangulationTest, DelaunayTriangulation_NotEnoughPoints) {
    std::vector<Point2D> points = {{0, 0}, {1, 1}};

    auto result = DelaunayTriangulation(points);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "At least three points are required for triangulation.");
}

TEST(TriangulationTest, DelaunayTriangulation_NoExcept) {
    std::vector<Point2D> points = {{0, 0}, {1, 0}, {0, 1}};
    EXPECT_NO_THROW({ auto result = DelaunayTriangulation(points); });
}

TEST(TriangulationTest, DelaunayTriangle_ContainsPoint) {
    DelaunayTriangle triangle{{0, 0}, {1, 0}, {0, 1}};

    Point2D inside{0.2, 0.2};
    Point2D on{1, 1};
    Point2D outside{1.5, 1.5};

    EXPECT_TRUE(triangle.ContainsPoint(inside));
    EXPECT_TRUE(triangle.ContainsPoint(on));
    EXPECT_FALSE(triangle.ContainsPoint(outside));
}