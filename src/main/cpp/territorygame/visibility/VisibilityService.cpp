#include "territorygame/visibility/VisibilityService.hpp"

#include <algorithm>

namespace territorygame::visibility {

using territorygame::api::GridPosition;
using territorygame::api::OccupantView;
using territorygame::api::TerritoryView;
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
            grid[y - minY][x - minX] = VisibleCell{
                position,
                classifyOccupant(board, position, viewer.getAgent(), opponent.getAgent(), viewerId, opponent.getId()),
                classifyTerritory(board, position, viewerId, opponent.getId())};
        }
    }
    return grid;
}

OccupantView VisibilityService::classifyOccupant(
    const Board& board, GridPosition position, const Agent& viewerAgent, const Agent& opponentAgent,
    PlayerId viewerId, PlayerId opponentId) const {
    if (position == viewerAgent.getPosition()) {
        return OccupantView::SELF_AGENT;
    }
    if (position == opponentAgent.getPosition()) {
        return OccupantView::OPPONENT_AGENT;
    }
    auto trailOwner = board.trailOwnerAt(position);
    if (trailOwner == viewerId) {
        return OccupantView::SELF_TRAIL;
    }
    if (trailOwner == opponentId) {
        return OccupantView::OPPONENT_TRAIL;
    }
    return OccupantView::EMPTY;
}

TerritoryView VisibilityService::classifyTerritory(
    const Board& board, GridPosition position, PlayerId viewerId, PlayerId opponentId) const {
    auto territoryOwner = board.territoryOwnerAt(position);
    if (territoryOwner == viewerId) {
        return TerritoryView::SELF;
    }
    if (territoryOwner == opponentId) {
        return TerritoryView::OPPONENT;
    }
    return TerritoryView::UNOWNED;
}

} // namespace territorygame::visibility
