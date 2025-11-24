#include "queries.hpp"
#include "shape_utils.hpp"
#include <gtest/gtest.h>

using namespace geometry;
using namespace geometry::utils;

// ========================================================
// Тесты парсинга отдельных фигур
// ========================================================

TEST(ShapeUtilsTest, ParseShapes_Circle) {
    auto shapes = ParseShapes("circle 0 0 1");
    ASSERT_EQ(shapes.size(), 1);
    ASSERT_TRUE(std::holds_alternative<Circle>(shapes[0]));
    const auto &c = std::get<Circle>(shapes[0]);
    EXPECT_EQ(c.center_p, Point2D(0, 0));
    EXPECT_DOUBLE_EQ(c.radius, 1.0);
}

TEST(ShapeUtilsTest, ParseShapes_Line) {
    auto shapes = ParseShapes("line 0 0 1 1");
    ASSERT_EQ(shapes.size(), 1);
    ASSERT_TRUE(std::holds_alternative<Line>(shapes[0]));
    const auto &l = std::get<Line>(shapes[0]);
    EXPECT_EQ(l.start, Point2D(0, 0));
    EXPECT_EQ(l.end, Point2D(1, 1));
}

TEST(ShapeUtilsTest, ParseShapes_Triangle) {
    auto shapes = ParseShapes("triangle 0 0 1 0 0 1");
    ASSERT_EQ(shapes.size(), 1);
    EXPECT_TRUE(std::holds_alternative<Triangle>(shapes[0]));
}

TEST(ShapeUtilsTest, ParseShapes_Rectangle) {
    auto shapes = ParseShapes("rectangle 0 0 2 3");
    ASSERT_EQ(shapes.size(), 1);
    ASSERT_TRUE(std::holds_alternative<Rectangle>(shapes[0]));
    const auto &r = std::get<Rectangle>(shapes[0]);
    EXPECT_EQ(r.bottom_left, Point2D(0, 0));
    EXPECT_DOUBLE_EQ(r.width, 2.0);
    EXPECT_DOUBLE_EQ(r.height, 3.0);
}

TEST(ShapeUtilsTest, ParseShapes_RegularPolygon) {
    auto shapes = ParseShapes("polygon 0 0 1 6");
    ASSERT_EQ(shapes.size(), 1);
    ASSERT_TRUE(std::holds_alternative<RegularPolygon>(shapes[0]));
    const auto &p = std::get<RegularPolygon>(shapes[0]);
    EXPECT_EQ(p.center_p, Point2D(0, 0));
    EXPECT_DOUBLE_EQ(p.radius, 1.0);
    EXPECT_EQ(p.sides, 6);
}

// ========================================================
// Тесты на ошибки парсинга
// ========================================================

TEST(ShapeUtilsTest, ParseShapes_InvalidPolygon_Skipped) {
    // polygon с 2 сторонами — недопустимо
    auto shapes = ParseShapes("polygon 0 0 1 2");
    EXPECT_TRUE(shapes.empty());  // недопустимая фигура пропущена
}

TEST(ShapeUtilsTest, ParseShapes_Mixed_ValidAndInvalid) {
    auto shapes = ParseShapes("circle 0 0 1; polygon 0 0 1 2; line 0 0 1 1");
    // Только circle и line должны быть распарсены
    ASSERT_EQ(shapes.size(), 2);
    EXPECT_TRUE(std::holds_alternative<Circle>(shapes[0]));
    EXPECT_TRUE(std::holds_alternative<Line>(shapes[1]));
}

TEST(ShapeUtilsTest, ParseShapes_InvalidNumbers) {
    auto shapes = ParseShapes("circle 0 abc 1");
    EXPECT_TRUE(shapes.empty());
}

TEST(ShapeUtilsTest, ParseShapes_EmptyInput) {
    auto shapes = ParseShapes("");
    EXPECT_TRUE(shapes.empty());
}

TEST(ShapeUtilsTest, ParseShapes_OnlySemicolons) {
    auto shapes = ParseShapes(";;;");
    EXPECT_TRUE(shapes.empty());
}

TEST(ShapeUtilsTest, ParseShapes_InvalidShapeType) {
    auto shapes = ParseShapes("invalid 1 2 3");
    EXPECT_TRUE(shapes.empty());
}

TEST(ShapeUtilsTest, ParseShapes_NegativeRadius) {
    auto shapes = ParseShapes("circle 0 0 -1");
    EXPECT_TRUE(shapes.empty());
}

TEST(ShapeUtilsTest, ParseShapes_ZeroDimensions) {
    auto shapes = ParseShapes("rectangle 0 0 0 1");
    EXPECT_TRUE(shapes.empty());
}

// ========================================================
// Тесты ParseShapes
// ========================================================

TEST(ShapeUtilsTest, ParseShapes_Multiple) {
    std::string input = "circle 0 0 1; line 0 0 1 1; rectangle 0 0 2 2";
    auto shapes = ParseShapes(input);
    ASSERT_EQ(shapes.size(), 3);
    EXPECT_TRUE(std::holds_alternative<Circle>(shapes[0]));
    EXPECT_TRUE(std::holds_alternative<Line>(shapes[1]));
    EXPECT_TRUE(std::holds_alternative<Rectangle>(shapes[2]));
}

TEST(ShapeUtilsTest, ParseShapes_WithSpaces) {
    std::string input = "  circle   0   0   1  ;  line  1 1 2 2  ";
    auto shapes = ParseShapes(input);
    ASSERT_EQ(shapes.size(), 2);
}

// ========================================================
// Тесты FindAllCollisions
// ========================================================

TEST(ShapeUtilsTest, FindAllCollisions_NoCollisions) {
    std::vector<Shape> shapes = {Circle{{0, 0}, 1}, Circle{{5, 5}, 1}};
    auto collisions = FindAllCollisions(shapes);
    EXPECT_TRUE(collisions.empty());
}

TEST(ShapeUtilsTest, FindAllCollisions_OneCollision) {
    std::vector<Shape> shapes = {Circle{{0, 0}, 2}, Circle{{1, 0}, 2}};
    auto collisions = FindAllCollisions(shapes);
    ASSERT_EQ(collisions.size(), 1);
    EXPECT_TRUE(std::holds_alternative<Circle>(collisions[0].first));
    EXPECT_TRUE(std::holds_alternative<Circle>(collisions[0].second));
}

TEST(ShapeUtilsTest, FindAllCollisions_Multiple) {
    std::vector<Shape> shapes = {
        Circle{{0, 0}, 1},    // 0
        Circle{{0.5, 0}, 1},  // 1 → пересекает 0
        Circle{{2.5, 0}, 1},  // 2 → пересекает 1
        Circle{{5, 5}, 1}     // 3 → нет
    };
    auto collisions = FindAllCollisions(shapes);
    // Ожидаем: (0,1), (1,2)
    ASSERT_EQ(collisions.size(), 2);
}

TEST(ShapeUtilsTest, FindAllCollisions_Empty) {
    std::vector<Shape> shapes;
    auto collisions = FindAllCollisions(shapes);
    EXPECT_TRUE(collisions.empty());
}

TEST(ShapeUtilsTest, FindAllCollisions_Single) {
    std::vector<Shape> shapes = {Circle{{0, 0}, 1}};
    auto collisions = FindAllCollisions(shapes);
    EXPECT_TRUE(collisions.empty());
}

TEST(ShapeUtilsTest, FindAllCollisions_DifferentShapes) {
    std::vector<Shape> shapes = {
        Circle{{0, 0}, 2}, Rectangle{{1, -1}, 2, 2}  // Пересекается с кругом
    };
    auto collisions = FindAllCollisions(shapes);
    ASSERT_EQ(collisions.size(), 1);
}

TEST(ShapeUtilsTest, FindAllCollisions_TouchingEdges) {
    // Касающиеся bounding boxes
    std::vector<Shape> shapes = {Rectangle{{0, 0}, 1, 1}, Rectangle{{1, 0}, 1, 1}};
    auto collisions = FindAllCollisions(shapes);
    ASSERT_EQ(collisions.size(), 1);
}

// ========================================================
// Тесты FindHighestShape
// ========================================================

TEST(ShapeUtilsTest, FindHighestShape_Single) {
    std::vector<Shape> shapes = {Circle{{0, 0}, 1}};
    auto idx = FindHighestShape(shapes);
    ASSERT_TRUE(idx.has_value());
    EXPECT_EQ(*idx, 0u);
}

TEST(ShapeUtilsTest, FindHighestShape_Multiple) {
    std::vector<Shape> shapes = {
        Circle{{0, 0}, 1},        // height = 1
        Rectangle{{0, 0}, 1, 3},  // height = 3
        Line{{0, 0}, {1, 2}}      // height = 2
    };
    auto idx = FindHighestShape(shapes);
    ASSERT_TRUE(idx.has_value());
    EXPECT_EQ(*idx, 1u);  // Rectangle — самый высокий
}

TEST(ShapeUtilsTest, FindHighestShape_Empty) {
    std::vector<Shape> shapes;
    EXPECT_FALSE(FindHighestShape(shapes).has_value());
}

TEST(ShapeUtilsTest, FindHighestShape_EqualHeights) {
    // Несколько фигур с одинаковой высотой - должен вернуть первую
    std::vector<Shape> shapes = {
        Circle{{0, 0}, 2},        // height = 2
        Rectangle{{0, 0}, 1, 2},  // height = 2
        Line{{0, 0}, {0, 2}}      // height = 2
    };
    auto idx = FindHighestShape(shapes);
    ASSERT_TRUE(idx.has_value());
    EXPECT_EQ(*idx, 0u);
}

TEST(ShapeUtilsTest, FindHighestShape_NegativeCoordinates) {
    std::vector<Shape> shapes = {
        Circle{{0, -5}, 1},       // height = -4
        Rectangle{{0, 0}, 1, 2},  // height = 2
        Line{{0, -10}, {0, -8}}   // height = -8
    };
    auto idx = FindHighestShape(shapes);
    ASSERT_TRUE(idx.has_value());
    EXPECT_EQ(*idx, 1u);  // Rectangle имеет наибольшую высоту
}

TEST(ShapeUtilsTest, FindHighestShape_ComplexShapes) {
    std::vector<Shape> shapes = {
        RegularPolygon{{0, 0}, 3, 6},                                  // height = 3
        Triangle{{0, 0}, {4, 0}, {2, 5}},                              // height = 5
        Polygon{std::vector<Point2D>{{0, 0}, {2, 0}, {2, 4}, {0, 4}}}  // height = 4
    };
    auto idx = FindHighestShape(shapes);
    ASSERT_TRUE(idx.has_value());
    EXPECT_EQ(*idx, 1u);
}

// ========================================================
// Тесты на корректность данных
// ========================================================

TEST(ShapeUtilsTest, ShapeIntegrityAfterParsing) {
    auto shapes = ParseShapes("circle 1.5 2.5 3.5; rectangle 0.5 1.5 2.5 3.5");
    ASSERT_EQ(shapes.size(), 2);

    // Проверяем что данные не искажены при парсинге
    const auto &circle = std::get<Circle>(shapes[0]);
    EXPECT_DOUBLE_EQ(circle.center_p.x, 1.5);
    EXPECT_DOUBLE_EQ(circle.center_p.y, 2.5);
    EXPECT_DOUBLE_EQ(circle.radius, 3.5);

    const auto &rect = std::get<Rectangle>(shapes[1]);
    EXPECT_DOUBLE_EQ(rect.bottom_left.x, 0.5);
    EXPECT_DOUBLE_EQ(rect.bottom_left.y, 1.5);
    EXPECT_DOUBLE_EQ(rect.width, 2.5);
    EXPECT_DOUBLE_EQ(rect.height, 3.5);
}
