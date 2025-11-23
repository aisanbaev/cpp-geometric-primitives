#include "shape_utils.hpp"
#include "queries.hpp"
#include <algorithm>
#include <functional>
#include <ranges>

namespace geometry::utils {
// Разбивает строку на слова (по пробелам), игнорируя лишние пробелы
std::vector<std::string_view> SplitIntoWords(std::string_view s) {
    std::vector<std::string_view> words;
    size_t start = 0;
    size_t end = 0;

    while (start < s.size()) {
        // Пропускаем пробелы
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
            ++start;
        if (start >= s.size())
            break;
        end = start;
        while (end < s.size() && !std::isspace(static_cast<unsigned char>(s[end])))
            ++end;
        words.push_back(s.substr(start, end - start));
        start = end;
    }
    return words;
}

// Безопасный парсинг строки в double (без исключений)
std::optional<double> ParseDouble(std::string_view s) {
    double value = 0.0;
    auto result = std::from_chars(s.data(), s.data() + s.size(), value);
    if (result.ec == std::errc{} && result.ptr == s.data() + s.size()) {
        return value;
    }
    return std::nullopt;
}

// Парсит строку в вектор double
std::optional<std::vector<double>> ParseDoubles(std::string_view s) {
    if (s.empty())
        return std::nullopt;
    auto tokens = SplitIntoWords(s);
    if (tokens.empty())
        return std::nullopt;

    std::vector<double> result;
    result.reserve(tokens.size());

    for (auto token : tokens) {
        auto num = ParseDouble(token);
        if (!num.has_value()) {
            return std::nullopt;
        }
        result.push_back(*num);
    }
    return result;
}

// Проверяет размер вектора и возвращает его, если совпадает
std::optional<std::vector<double>> RequireSize(const std::vector<double> &v, size_t expected) {
    return (v.size() == expected) ? std::make_optional(v) : std::nullopt;
}

// Проверяет, что значение > 0
std::optional<double> RequirePositive(double x) { return (x > 0) ? std::make_optional(x) : std::nullopt; }

// Проверяет, что double представляет целое число >= min_value
std::optional<int> RequireIntegerAtLeast(double d, int min_value) {
    int i = static_cast<int>(d);
    if (static_cast<double>(i) == d && i >= min_value) {
        return i;
    }
    return std::nullopt;
}

// Конструкторы фигур

/**
    @brief Создаёт круг из параметров
*/
std::optional<Shape> MakeCircle(const std::vector<double> &v) {
    return RequireSize(v, 3)
        .and_then([](const auto &vec) { return RequirePositive(vec[2]); })
        .transform([&v](double radius) { return Circle{{v[0], v[1]}, radius}; });
}

/**
    @brief Создаёт линию из параметров
*/
std::optional<Shape> MakeLine(const std::vector<double> &v) {
    return RequireSize(v, 4).transform([&v](const auto &vec) { return Line{{v[0], v[1]}, {v[2], v[3]}}; });
}

/**
    @brief Создаёт треугольник из параметров
*/
std::optional<Shape> MakeTriangle(const std::vector<double> &v) {
    return RequireSize(v, 6).transform(
        [&v](const auto &vec) { return Triangle{{v[0], v[1]}, {v[2], v[3]}, {v[4], v[5]}}; });
}

/**
    @brief Создаёт прямоугольник из параметров
*/
std::optional<Shape> MakeRectangle(const std::vector<double> &v) {
    return RequireSize(v, 4)
        .and_then([&v](const auto &vec) {
            return RequirePositive(v[2]).and_then([&v](double width) {
                return RequirePositive(v[3]).transform(
                    [&v, width](double height) { return std::make_pair(width, height); });
            });
        })
        .transform([&v](const auto &dims) { return Rectangle{{v[0], v[1]}, dims.first, dims.second}; });
}

/**
    @brief Создаёт правильный многоугольник из параметров
*/
std::optional<Shape> MakePolygon(const std::vector<double> &v) {
    return RequireSize(v, 4)
        .and_then([&v](const auto &vec) {
            return RequirePositive(v[2]).and_then([&v](double radius) {
                return RequireIntegerAtLeast(v[3], 3).transform(
                    [&v, radius](int sides) { return std::make_pair(radius, sides); });
            });
        })
        .transform([&v](const auto &params) { return RegularPolygon{{v[0], v[1]}, params.first, params.second}; });
}

// Парсинг одной фигуры
std::optional<Shape> ParseSingleShape(std::string_view token) {
    auto parts = SplitIntoWords(token);
    if (parts.empty())
        return std::nullopt;

    std::string_view type = parts[0];
    std::string param_str;
    for (auto i : std::views::iota(1u, parts.size())) {
        if (!param_str.empty())
            param_str += ' ';
        param_str += std::string(parts[i]);
    }

    // Выбираем конструктор по имени
    auto get_maker =
        [](std::string_view t) -> std::optional<std::function<std::optional<Shape>(const std::vector<double> &)>> {
        if (t == "circle")
            return MakeCircle;
        if (t == "line")
            return MakeLine;
        if (t == "triangle")
            return MakeTriangle;
        if (t == "rectangle")
            return MakeRectangle;
        if (t == "polygon")
            return MakePolygon;
        return std::nullopt;
    };

    // Обратите внимание на код ниже
    return get_maker(type).and_then([&](auto maker) { return ParseDoubles(param_str).and_then(maker); });
}

std::vector<Shape> ParseShapes(std::string_view input) {
    std::vector<Shape> result;

    // Разделяем по ';'
    size_t start = 0;
    size_t end = 0;
    while (start < input.size()) {
        end = input.find(';', start);
        if (end == std::string_view::npos)
            end = input.size();

        std::string_view token = input.substr(start, end - start);
        // Убираем пробелы по краям
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front())))
            token.remove_prefix(1);
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back())))
            token.remove_suffix(1);

        if (!token.empty()) {
            auto shape_opt = ParseSingleShape(token);
            if (shape_opt.has_value()) {
                result.push_back(*shape_opt);
            }
        }

        start = end + 1;
    }

    return result;
}

std::vector<std::pair<Shape, Shape>> FindAllCollisions(std::span<const Shape> shapes) {
    std::vector<std::pair<Shape, Shape>> collisions;

    if (shapes.size() < 2)
        return collisions;

    auto indices = std::views::iota(0uz, shapes.size());

    auto all_pairs = indices | std::views::transform([&](size_t i) {
                         return std::views::iota(i + 1, shapes.size()) |
                                std::views::transform([&, i](size_t j) { return std::make_pair(i, j); });
                     }) |
                     std::views::join;

    std::ranges::for_each(all_pairs, [&](const auto &pair) {
        const auto &[i, j] = pair;
        if (geometry::queries::BoundingBoxesOverlap(shapes[i], shapes[j])) {
            collisions.emplace_back(shapes[i], shapes[j]);
        }
    });

    return collisions;
}

std::optional<size_t> FindHighestShape(std::span<const Shape> shapes) {
    using namespace geometry::queries;
    if (shapes.empty())
        return std::nullopt;

    auto indices = std::views::iota(0uz, shapes.size());

    auto max_index_it = std::ranges::max_element(
        indices, [&](size_t i, size_t j) { return GetHeight(shapes[i]) < GetHeight(shapes[j]); });

    return *max_index_it;
}

}  // namespace geometry::utils