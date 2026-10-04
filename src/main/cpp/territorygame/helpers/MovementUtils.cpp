#include "territorygame/helpers/MovementUtils.hpp"

#include <cstdlib>
#include <stdexcept>

namespace territorygame::helpers {

using namespace territorygame::api;

GridPosition MovementUtils::nextPosition(GridPosition position, Direction direction) {
    switch (direction) {
        case Direction::NORTH: return GridPosition{position.x, position.y - 1};
        case Direction::SOUTH: return GridPosition{position.x, position.y + 1};
        case Direction::EAST: return GridPosition{position.x + 1, position.y};
        case Direction::WEST: return GridPosition{position.x - 1, position.y};
    }
    throw std::invalid_argument("Unknown direction");
}

bool MovementUtils::isWithinBoard(GridPosition position, int width, int height) {
    return position.x >= 0 && position.x < width && position.y >= 0 && position.y < height;
}

bool MovementUtils::isValidBoardMove(GridPosition position, Direction direction, int width, int height) {
    return isWithinBoard(nextPosition(position, direction), width, height);
}

int MovementUtils::manhattanDistance(GridPosition a, GridPosition b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool MovementUtils::isValidMove(const GameApi& game, Direction direction) {
    GridPosition destination = nextPosition(game.getAgentPosition(), direction);
    if (!isWithinBoard(destination, game.getBoardWidth(), game.getBoardHeight())) {
        return false;
    }
    // Visibility radius always covers adjacent cells, so an absent cell
    // here is unreachable in practice; bounds are already confirmed above.
    auto cell = findCell(game.getVisibleGrid(), destination);
    if (!cell.has_value()) {
        return true;
    }
    return cell->occupant != OccupantView::OPPONENT_AGENT;
}

std::optional<VisibleCell> MovementUtils::findCell(
    const std::vector<std::vector<VisibleCell>>& visibleGrid, GridPosition position) {
    for (const auto& row : visibleGrid) {
        for (const auto& cell : row) {
            if (cell.position == position) {
                return cell;
            }
        }
    }
    return std::nullopt;
}

std::vector<Direction> MovementUtils::validDirections(const GameApi& game) {
    std::vector<Direction> directions;
    for (Direction direction : DIRECTIONS) {
        if (isValidMove(game, direction)) {
            directions.push_back(direction);
        }
    }
    return directions;
}

Direction MovementUtils::randomDirection(std::mt19937_64& random) {
    std::uniform_int_distribution<size_t> dist(0, DIRECTIONS.size() - 1);
    return DIRECTIONS[dist(random)];
}

} // namespace territorygame::helpers
