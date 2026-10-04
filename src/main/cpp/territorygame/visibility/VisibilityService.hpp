#pragma once

#include <vector>

#include "territorygame/api/GridPosition.hpp"
#include "territorygame/api/OccupantView.hpp"
#include "territorygame/api/TerritoryView.hpp"
#include "territorygame/api/VisibleCell.hpp"
#include "territorygame/domain/Agent.hpp"
#include "territorygame/domain/Board.hpp"
#include "territorygame/domain/GameState.hpp"
#include "territorygame/domain/PlayerId.hpp"

namespace territorygame::visibility {

// Produces candidate-facing observations from authoritative state, owning
// the translation from internal player identities to relative SELF/OPPONENT
// occupant and territory views.
class VisibilityService {
public:
    explicit VisibilityService(int windowSize);

    // Returns a window of cells centered on viewerId's agent, clipped to
    // the board, indexed [row][column] (y, then x).
    std::vector<std::vector<territorygame::api::VisibleCell>> computeVisibleGrid(
        const territorygame::domain::GameState& state, territorygame::domain::PlayerId viewerId) const;

private:
    territorygame::api::OccupantView classifyOccupant(
        const territorygame::domain::Board& board, territorygame::api::GridPosition position,
        const territorygame::domain::Agent& viewerAgent, const territorygame::domain::Agent& opponentAgent,
        territorygame::domain::PlayerId viewerId, territorygame::domain::PlayerId opponentId) const;

    territorygame::api::TerritoryView classifyTerritory(
        const territorygame::domain::Board& board, territorygame::api::GridPosition position,
        territorygame::domain::PlayerId viewerId, territorygame::domain::PlayerId opponentId) const;

    int windowSize_;
};

} // namespace territorygame::visibility
