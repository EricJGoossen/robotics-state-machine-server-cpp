#include "territorygame/rules/TerritoryResolver.hpp"

#include "territorygame/helpers/MovementUtils.hpp"

namespace territorygame::rules {

using territorygame::api::Direction;
using territorygame::api::GridPosition;
using territorygame::api::DIRECTIONS;
using territorygame::domain::Agent;
using territorygame::domain::Board;
using territorygame::domain::GameState;
using territorygame::domain::Player;
using territorygame::domain::PlayerId;
using territorygame::helpers::MovementUtils;

void TerritoryResolver::applyCapture(GameState& state, PlayerId capturerId) {
    Board& board = state.getBoard();
    Player& capturer = state.getPlayer(capturerId);
    Agent& agent = capturer.getAgent();

    auto trail = agent.getActiveTrail();
    for (const auto& cell : trail) {
        board.setTerritoryOwner(cell, capturerId);
        board.setTrailOwner(cell, std::nullopt);
    }

    for (const auto& enclosedCell : findEnclosedCells(board, capturerId)) {
        board.setTerritoryOwner(enclosedCell, capturerId);
    }

    restoreOpponentStartingTerritories(state, capturerId);

    agent.clearTrail();
}

void TerritoryResolver::restoreOpponentStartingTerritories(GameState& state, PlayerId capturerId) {
    Board& board = state.getBoard();
    for (Player& player : state.getPlayers()) {
        if (player.getId() == capturerId) {
            continue;
        }
        for (const auto& cell : player.getStartingTerritory()) {
            board.setTerritoryOwner(cell, player.getId());
        }
    }
}

std::vector<GridPosition> TerritoryResolver::findEnclosedCells(Board& board, PlayerId capturerId) {
    int width = board.getWidth();
    int height = board.getHeight();
    std::vector<std::vector<bool>> reachedFromEdge(height, std::vector<bool>(width, false));
    std::deque<GridPosition> frontier;

    for (int x = 0; x < width; x++) {
        seedIfOutsideTerritory(board, capturerId, GridPosition{x, 0}, reachedFromEdge, frontier);
        seedIfOutsideTerritory(board, capturerId, GridPosition{x, height - 1}, reachedFromEdge, frontier);
    }
    for (int y = 0; y < height; y++) {
        seedIfOutsideTerritory(board, capturerId, GridPosition{0, y}, reachedFromEdge, frontier);
        seedIfOutsideTerritory(board, capturerId, GridPosition{width - 1, y}, reachedFromEdge, frontier);
    }

    while (!frontier.empty()) {
        GridPosition current = frontier.front();
        frontier.pop_front();
        for (const auto& neighbor : cardinalNeighbors(current, width, height)) {
            if (!reachedFromEdge[neighbor.y][neighbor.x]
                    && board.territoryOwnerAt(neighbor) != capturerId) {
                reachedFromEdge[neighbor.y][neighbor.x] = true;
                frontier.push_back(neighbor);
            }
        }
    }

    std::vector<GridPosition> enclosed;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            GridPosition position{x, y};
            if (!reachedFromEdge[y][x] && board.territoryOwnerAt(position) != capturerId) {
                enclosed.push_back(position);
            }
        }
    }
    return enclosed;
}

void TerritoryResolver::seedIfOutsideTerritory(
    Board& board, PlayerId capturerId, GridPosition position,
    std::vector<std::vector<bool>>& reachedFromEdge, std::deque<GridPosition>& frontier) {
    if (board.territoryOwnerAt(position) != capturerId && !reachedFromEdge[position.y][position.x]) {
        reachedFromEdge[position.y][position.x] = true;
        frontier.push_back(position);
    }
}

std::vector<GridPosition> TerritoryResolver::cardinalNeighbors(GridPosition position, int width, int height) {
    std::vector<GridPosition> neighbors;
    neighbors.reserve(4);
    for (Direction direction : DIRECTIONS) {
        GridPosition neighbor = MovementUtils::nextPosition(position, direction);
        if (MovementUtils::isWithinBoard(neighbor, width, height)) {
            neighbors.push_back(neighbor);
        }
    }
    return neighbors;
}

} // namespace territorygame::rules
