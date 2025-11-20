#include "geometry.hpp"
#include <gtest/gtest.h>

using namespace geometry;

// ----------------------------
// Point2D tests
// ----------------------------

TEST(Point2DTest, ConstructionAndAccess) {
    Point2D p{1.5, -2.0};
    EXPECT_EQ(p.x, 1.5);
    EXPECT_EQ(p.y, -2.0);
}

TEST(Point2DTest, Operators) {
    Point2D a{1, 2};
    Point2D b{3, 4};

    EXPECT_EQ(a + b, (Point2D{4, 6}));
    EXPECT_EQ(b - a, (Point2D{2, 2}));
    EXPECT_EQ(a * 2.0, (Point2D{2, 4}));
    EXPECT_EQ(b / 2.0, (Point2D{1.5, 2.0}));

    EXPECT_DOUBLE_EQ(a.Dot(b), 1 * 3 + 2 * 4);    // 11
    EXPECT_DOUBLE_EQ(a.Cross(b), 1 * 4 - 2 * 3);  // -2
}

TEST(Point2DTest, LengthAndNormalize) {
    Point2D p{3, 4};
    EXPECT_DOUBLE_EQ(p.Length(), 5.0);
    Point2D n = p.Normalize();
    EXPECT_NEAR(n.Length(), 1.0, 1e-9);
}

TEST(Point2DTest, DistanceTo) {
    Point2D a{0, 0};
    Point2D b{3, 4};
    EXPECT_DOUBLE_EQ(a.DistanceTo(b), 5.0);
}

// ----------------------------
// Formatting tests
// ----------------------------

TEST(FormatterTest, Point2DFormat) {
    Point2D p{1.234, -5.678};
    EXPECT_EQ(std::format("{}", p), "(1.23, -5.68)");
}

TEST(FormatterTest, VectorPoint2D_Line) {
    std::vector<Point2D> pts{{0, 0}, {1, 1}, {2, 0}};
    std::string expected = "(0.00, 0.00) (1.00, 1.00) (2.00, 0.00)";
    EXPECT_EQ(std::format("{}", pts), expected);
}

TEST(FormatterTest, VectorPoint2D_NewLine) {
    std::vector<Point2D> pts{{0, 0}, {1, 1}, {2, 0}};
    std::string expected = "\t(0.00, 0.00)\n\t(1.00, 1.00)\n\t(2.00, 0.00)\n";
    EXPECT_EQ(std::format("{:new_line}", pts), expected);
}

TEST(FormatterTest, VectorPoint2D_Empty) {
    std::vector<Point2D> empty;
    EXPECT_EQ(std::format("{}", empty), "");
    EXPECT_EQ(std::format("{:new_line}", empty), "");
}

TEST(FormatterTest, LineFormatting) {
    Line line{Point2D{0.0, 0.0}, Point2D{3.0, 4.0}};
    EXPECT_EQ(std::format("{}", line), "Line((0.00, 0.00), (3.00, 4.00))");
}

TEST(FormatterTest, CircleFormatting) {
    Circle circle{Point2D{2.0, 3.0}, 5.0};
    EXPECT_EQ(std::format("{}", circle), "Circle(center=(2.00, 3.00), r=5.00)");
}

TEST(FormatterTest, RectangleFormatting) {
    Rectangle rect{Point2D{1.0, 2.0}, 4.0, 3.0};
    EXPECT_EQ(std::format("{}", rect), "Rectangle(bottom_left=(1.00, 2.00), w=4.00, h=3.00)");
}

TEST(FormatterTest, RegularPolygonFormatting) {
    RegularPolygon hexagon{Point2D{0.0, 0.0}, 5.0, 6};
    EXPECT_EQ(std::format("{}", hexagon), "RegularPolygon(center=(0.00, 0.00), r=5.00, sides=6)");
}

TEST(FormatterTest, TriangleFormatting) {
    Triangle tri{Point2D{0.0, 0.0}, Point2D{4.0, 0.0}, Point2D{0.0, 3.0}};
    EXPECT_EQ(std::format("{}", tri), "Triangle((0.00, 0.00), (4.00, 0.00), (0.00, 3.00))");
}

TEST(FormatterTest, PolygonFormatting) {
    // Полигон с одной точкой
    Polygon single_point_poly{std::vector<Point2D>{{1.0, 2.0}}};
    EXPECT_EQ(std::format("{}", single_point_poly), "Polygon[1 points]: [(1.00, 2.00) ]");

    // Полигон с несколькими точками
    std::vector<Point2D> quad_points{{0.0, 0.0}, {4.0, 0.0}, {4.0, 3.0}, {0.0, 3.0}};
    Polygon quad_poly{quad_points};

    std::string result = std::format("{}", quad_poly);
    EXPECT_TRUE(result.starts_with("Polygon[4 points]: ["));
    EXPECT_TRUE(result.find("(0.00, 0.00)") != std::string::npos);
    EXPECT_TRUE(result.find("(4.00, 0.00)") != std::string::npos);
    EXPECT_TRUE(result.find("(4.00, 3.00)") != std::string::npos);
    EXPECT_TRUE(result.find("(0.00, 3.00)") != std::string::npos);
    EXPECT_TRUE(result.ends_with("]"));
}

// ----------------------------
// Primitive tests
// ----------------------------

TEST(LineTest, BasicProperties) {
    Line line{{0, 0}, {3, 4}};
    EXPECT_DOUBLE_EQ(line.Length(), 5.0);
    EXPECT_EQ(line.Vertices(), (std::array{Point2D{0, 0}, Point2D{3, 4}}));
    BoundingBox bb = line.BoundBox();
    EXPECT_EQ(bb.min_x, 0);
    EXPECT_EQ(bb.min_y, 0);
    EXPECT_EQ(bb.max_x, 3);
    EXPECT_EQ(bb.max_y, 4);
}

TEST(TriangleTest, AreaAndBoundingBox) {
    Triangle tri{{0, 0}, {1, 0}, {0, 1}};
    EXPECT_DOUBLE_EQ(tri.Area(), 0.5);
    BoundingBox bb = tri.BoundBox();
    EXPECT_EQ(bb.min_x, 0);
    EXPECT_EQ(bb.min_y, 0);
    EXPECT_EQ(bb.max_x, 1);
    EXPECT_EQ(bb.max_y, 1);
}

TEST(CircleTest, BoundingBoxAndVertices) {
    Circle c{{1, 1}, 2};
    BoundingBox bb = c.BoundBox();
    EXPECT_EQ(bb.min_x, -1);
    EXPECT_EQ(bb.min_y, -1);
    EXPECT_EQ(bb.max_x, 3);
    EXPECT_EQ(bb.max_y, 3);

    auto verts = c.Vertices(4);
    ASSERT_EQ(verts.size(), 4);
    // первый — (1+2, 1) = (3,1)
    EXPECT_NEAR(verts[0].x, 3.0, 1e-6);
    EXPECT_NEAR(verts[0].y, 1.0, 1e-6);
}

TEST(PolygonTest, CustomPolygon) {
    std::vector<Point2D> pts{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    Polygon poly{pts};

    BoundingBox bb = poly.BoundBox();
    EXPECT_EQ(bb.min_x, 0);
    EXPECT_EQ(bb.min_y, 0);
    EXPECT_EQ(bb.max_x, 2);
    EXPECT_EQ(bb.max_y, 2);

    EXPECT_EQ(poly.Vertices().size(), 4);
    EXPECT_EQ(poly.Center(), (Point2D{1, 1}));
}

TEST(BoundingBoxTest, Overlaps) {
    BoundingBox a{0, 0, 2, 2};
    BoundingBox b{1, 1, 3, 3};
    BoundingBox c{3, 3, 5, 5};

    EXPECT_TRUE(a.Overlaps(b));
    EXPECT_FALSE(a.Overlaps(c));
}

// ----------------------------
// Shape variant (sanity check)
// ----------------------------

TEST(ShapeTest, VariantConstruction) {
    Shape s1 = Line{{0, 0}, {1, 1}};
    Shape s2 = Circle{{0, 0}, 1.0};
    Shape s3 = Polygon{{Point2D{0, 0}, {1, 0}, {0, 1}}};

    // просто проверяем, что компилируется и хранится
    EXPECT_TRUE(std::holds_alternative<Line>(s1));
    EXPECT_TRUE(std::holds_alternative<Circle>(s2));
    EXPECT_TRUE(std::holds_alternative<Polygon>(s3));
}