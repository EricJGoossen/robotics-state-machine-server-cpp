#include "territorygame/rules/RespawnService.hpp"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <vector>

#include "territorygame/helpers/MovementUtils.hpp"

namespace territorygame::rules {

using territorygame::api::GridPosition;
using territorygame::domain::Board;
using territorygame::domain::GameState;
using territorygame::domain::Player;
using territorygame::domain::PlayerId;
using territorygame::helpers::MovementUtils;

namespace {

std::vector<GridPosition> allPositions(int width, int height) {
    std::vector<GridPosition> positions;
    positions.reserve(static_cast<size_t>(width) * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            positions.push_back(GridPosition{x, y});
        }
    }
    return positions;
}

bool nearestToRespawnLess(const GridPosition& a, const GridPosition& b, GridPosition respawnPosition) {
    int distA = MovementUtils::manhattanDistance(a, respawnPosition);
    int distB = MovementUtils::manhattanDistance(b, respawnPosition);
    if (distA != distB) return distA < distB;
    if (a.y != b.y) return a.y < b.y;
    return a.x < b.x;
}

} // namespace

void RespawnService::respawn(GameState& state, PlayerId playerId) {
    Board& board = state.getBoard();
    Player& player = state.getPlayer(playerId);
    auto& agent = player.getAgent();

    for (const auto& trailCell : agent.getActiveTrail()) {
        board.setTrailOwner(trailCell, std::nullopt);
    }
    agent.clearTrail();

    board.clearAllTerritoryOf(playerId);
    for (const auto& cell : player.getStartingTerritory()) {
        board.setTerritoryOwner(cell, playerId);
    }

    agent.setPosition(choosePosition(state, playerId));
}

GridPosition RespawnService::choosePosition(GameState& state, PlayerId playerId) {
    Player& player = state.getPlayer(playerId);
    GridPosition respawnPosition = player.getAgent().getRespawnPosition();
    GridPosition opponentPosition = state.getOpponent(playerId).getAgent().getPosition();

    if (!(respawnPosition == opponentPosition)) {
        return respawnPosition;
    }

    const auto& startingTerritory = player.getStartingTerritory();
    const GridPosition* best = nullptr;
    for (const auto& cell : startingTerritory) {
        if (cell == opponentPosition) {
            continue;
        }
        if (best == nullptr || nearestToRespawnLess(cell, *best, respawnPosition)) {
            best = &cell;
        }
    }
    if (best != nullptr) {
        return *best;
    }

    // Starting territory is fully blocked (e.g. a 1-cell starting territory
    // with the opponent standing on it): fall back to the nearest free cell
    // anywhere on the board rather than failing.
    Board& board = state.getBoard();
    auto candidates = allPositions(board.getWidth(), board.getHeight());
    const GridPosition* bestOverall = nullptr;
    for (const auto& cell : candidates) {
        if (cell == opponentPosition) {
            continue;
        }
        if (bestOverall == nullptr || nearestToRespawnLess(cell, *bestOverall, respawnPosition)) {
            bestOverall = &cell;
        }
    }
    if (bestOverall == nullptr) {
        throw std::runtime_error("No unoccupied cell available anywhere on the board");
    }
    return *bestOverall;
}

} // namespace territorygame::rules
