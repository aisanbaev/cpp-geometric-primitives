#include "visualization.hpp"
#include "geometry.hpp"

#include <matplot/matplot.h>
#include <print>

namespace geometry::visualization {

template <class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};

auto DrawConfig() {
    using namespace geometry;
    using namespace matplot;

    // Disable gnuplot warnings
    auto f = figure(false);
    f->backend()->run_command("unset warnings");
    f->ioff();
    f->size(900, 900);

    hold(on);     // Multiple plots mode
    axis(equal);  // Squre view
    grid(on);     // Enable grid by default
    xlim({-6, 15});
    ylim({-6, 15});
    return f;
}

void Draw(std::span<geometry::Shape> shapes, std::string_view filename) {
    using namespace geometry;
    using namespace matplot;

    const auto &fh = DrawConfig();

    for (const auto &[index, shape] : std::ranges::views::enumerate(shapes)) {
        std::visit(Multilambda{[index](const Line &line) {
                                   auto lines = line.Lines();
                                   auto p = plot(lines.x, lines.y);
                                   p->line_width(2).color("yellow");
                                   std::println("Drawing Line {}: {} -> {}", index, line.start, line.end);
                               },
                               [index](const Triangle &triangle) {
                                   auto lines = triangle.Lines();
                                   auto p = plot(lines.x, lines.y);
                                   p->line_width(2).color("blue");
                                   std::println("Drawing Triangle {}: {}, {}, {}", index, triangle.a, triangle.b,
                                                triangle.c);
                               },
                               [index](const Rectangle &rect) {
                                   auto lines = rect.Lines();
                                   auto p = plot(lines.x, lines.y);
                                   p->line_width(2).color("green");
                                   std::println("Drawing Rectangle {}: bottom_left={}, width={:.2f}, height={:.2f}",
                                                index, rect.bottom_left, rect.width, rect.height);
                               },
                               [index](RegularPolygon poly) {
                                   auto lines = poly.Lines();
                                   auto p = plot(lines.x, lines.y);
                                   p->line_width(2).color("magenta");
                                   std::println("Drawing RegularPolygon {}: center={}, radius={:.2f}, sides={}", index,
                                                poly.center_p, poly.radius, poly.sides);
                               },
                               [index](Circle circle) {
                                   auto lines = circle.Lines();
                                   auto p = plot(lines.x, lines.y);
                                   p->line_width(2).color("red");
                                   std::println("Drawing Circle {}: center={}, radius={:.2f}", index, circle.center_p,
                                                circle.radius);
                               },
                               [index](const Polygon &poly) {
                                   auto lines = poly.Lines();
                                   auto p = plot(lines.x, lines.y);
                                   p->line_width(2).color("cyan");
                                   std::println("Drawing Polygon {} with {} vertices", index, poly.Vertices().size());
                               }},
                   shape);

        auto center = std::visit([](const auto &s) -> Point2D { return s.Center(); }, shape);

        auto t = matplot::text(center.x, center.y, std::to_string(index));
        t->font_size(14);
        t->color("black");
    }

    fh->save(std::string(filename));
    std::println("Saved plot to {}", filename);
}

void Draw(std::span<const geometry::triangulation::DelaunayTriangle> triangles, std::string_view filename) {
    using namespace geometry;
    using namespace matplot;

    const auto &fh = DrawConfig();

    for (const auto &[index, d_triangle] : std::ranges::views::enumerate(triangles)) {
        const geometry::Triangle tri{d_triangle.a, d_triangle.b, d_triangle.c};
        const auto lines = tri.Lines();
        plot(lines.x, lines.y)->line_width(2).color("cyan");

        const auto center = tri.Center();
        auto t = text(center.x, center.y, std::to_string(index));
        t->font_size(14);
        t->color("black");
    }

    fh->save(std::string(filename));
    std::println("Saved triangulation to {}", filename);
}

}  // namespace geometry::visualization