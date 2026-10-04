#pragma once

#include <optional>
#include <random>
#include <vector>

#include "territorygame/api/Direction.hpp"
#include "territorygame/api/GameApi.hpp"
#include "territorygame/api/GridPosition.hpp"
#include "territorygame/api/VisibleCell.hpp"

namespace territorygame::helpers {

// Pure, stateless helpers for reasoning about movement and board bounds.
class MovementUtils {
public:
    MovementUtils() = delete;

    static territorygame::api::GridPosition nextPosition(
        territorygame::api::GridPosition position, territorygame::api::Direction direction);

    static bool isWithinBoard(territorygame::api::GridPosition position, int width, int height);

    static bool isValidBoardMove(
        territorygame::api::GridPosition position, territorygame::api::Direction direction, int width, int height);

    // Grid (non-diagonal) distance between two positions.
    static int manhattanDistance(territorygame::api::GridPosition a, territorygame::api::GridPosition b);

    // Checks the two mechanical invalid-move rules available before
    // submission: board bounds and whether the adjacent destination is
    // currently the opponent's agent. Makes no strategic decision and picks
    // no alternative direction. Notably, still allows moving onto the
    // player's own trail.
    static bool isValidMove(const territorygame::api::GameApi& game, territorygame::api::Direction direction);

    // Finds the visible cell at an absolute position, if it's within the visible window.
    static std::optional<territorygame::api::VisibleCell> findCell(
        const std::vector<std::vector<territorygame::api::VisibleCell>>& visibleGrid,
        territorygame::api::GridPosition position);

    // Directions that are mechanically valid right now: in bounds and not onto
    // the opponent's agent. Still includes moves onto the player's own trail.
    static std::vector<territorygame::api::Direction> validDirections(const territorygame::api::GameApi& game);

    // Picks a uniformly random direction.
    static territorygame::api::Direction randomDirection(std::mt19937_64& random);
};

} // namespace territorygame::helpers
