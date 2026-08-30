#include "territorygame/visibility/VisibilityService.hpp"

#include <algorithm>

namespace territorygame::visibility {

using territorygame::api::CellViewType;
using territorygame::api::GridPosition;
using territorygame::api::VisibleCell;
using territorygame::domain::Agent;
using territorygame::domain::Board;
using territorygame::domain::GameState;
using territorygame::domain::Player;
using territorygame::domain::PlayerId;

VisibilityService::VisibilityService(int windowSize) : windowSize_(windowSize) {}

std::vector<std::vector<VisibleCell>> VisibilityService::computeVisibleGrid(
    const GameState& state, PlayerId viewerId) const {
    const Board& board = state.getBoard();
    const Player& viewer = state.getPlayer(viewerId);
    const Player& opponent = state.getOpponent(viewerId);
    GridPosition center = viewer.getAgent().getPosition();

    int half = windowSize_ / 2;
    int minX = std::max(0, center.x - half);
    int maxX = std::min(board.getWidth() - 1, center.x + half);
    int minY = std::max(0, center.y - half);
    int maxY = std::min(board.getHeight() - 1, center.y + half);

    std::vector<std::vector<VisibleCell>> grid(
        maxY - minY + 1, std::vector<VisibleCell>(maxX - minX + 1));
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            GridPosition position{x, y};
            CellViewType type = classify(
                board, position, viewer.getAgent(), opponent.getAgent(), viewerId, opponent.getId());
            grid[y - minY][x - minX] = VisibleCell{position, type};
        }
    }
    return grid;
}

CellViewType VisibilityService::classify(
    const Board& board, GridPosition position, const Agent& viewerAgent, const Agent& opponentAgent,
    PlayerId viewerId, PlayerId opponentId) const {
    if (position == viewerAgent.getPosition()) {
        return CellViewType::SELF_AGENT;
    }
    if (position == opponentAgent.getPosition()) {
        return CellViewType::OPPONENT_AGENT;
    }

    auto trailOwner = board.trailOwnerAt(position);
    if (trailOwner == viewerId) {
        return CellViewType::SELF_TRAIL;
    }
    if (trailOwner == opponentId) {
        return CellViewType::OPPONENT_TRAIL;
    }

    auto territoryOwner = board.territoryOwnerAt(position);
    if (territoryOwner == viewerId) {
        return CellViewType::SELF_TERRITORY;
    }
    if (territoryOwner == opponentId) {
        return CellViewType::OPPONENT_TERRITORY;
    }

    return CellViewType::FREE;
}

} // namespace territorygame::visibility
