#include "convex_hull.hpp"
#include <gtest/gtest.h>

using namespace geometry;
using namespace geometry::convex_hull;

TEST(ConvexHullTest, GrahamScan_Valid) {
    std::vector<Point2D> points = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0.5, 0.5}};
    auto result = geometry::convex_hull::GrahamScan(points);
    ASSERT_TRUE(result.has_value());
    EXPECT_GE(result->size(), 3);
}

TEST(ConvexHullTest, GrahamScan_TooFewPoints) {
    std::vector<Point2D> points = {{0, 0}, {1, 0}};
    auto result = geometry::convex_hull::GrahamScan(points);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "At least three points are required for convex hull.");
}