#include "geometry.hpp"
#include "triangulation.hpp"
#include <span>

namespace geometry::visualization {

void Draw(std::span<geometry::Shape> shapes, std::string_view filename);

void Draw(std::span<const geometry::triangulation::DelaunayTriangle> triangles, std::string_view filename);

}  // namespace geometry::visualization
