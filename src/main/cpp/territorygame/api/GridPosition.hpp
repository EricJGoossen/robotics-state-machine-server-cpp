#pragma once

#include <cstddef>
#include <functional>

namespace territorygame::api {

/**
 * An absolute board coordinate. (0, 0) is top-left; x increases right,
 * y increases down.
 */
struct GridPosition {
    int x;
    int y;

    bool operator==(const GridPosition& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const GridPosition& other) const {
        return !(*this == other);
    }
};

} // namespace territorygame::api

namespace std {
template <>
struct hash<territorygame::api::GridPosition> {
    size_t operator()(const territorygame::api::GridPosition& p) const noexcept {
        size_t h1 = std::hash<int>()(p.x);
        size_t h2 = std::hash<int>()(p.y);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
} // namespace std
