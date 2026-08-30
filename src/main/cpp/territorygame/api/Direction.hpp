#pragma once

#include <array>

namespace territorygame::api {

/** The four cardinal movement directions an agent can take. */
enum class Direction { NORTH, SOUTH, EAST, WEST };

inline constexpr std::array<Direction, 4> DIRECTIONS {
    Direction::NORTH, Direction::SOUTH, Direction::EAST, Direction::WEST};

} // namespace territorygame::api
