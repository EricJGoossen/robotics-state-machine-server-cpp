#pragma once

#include <deque>
#include <vector>

#include "territorygame/api/GridPosition.hpp"
#include "territorygame/domain/Board.hpp"
#include "territorygame/domain/GameState.hpp"
#include "territorygame/domain/PlayerId.hpp"

namespace territorygame::rules {

// Applies capture effects when a player's trail closes: converting the
// trail to territory and flood-filling the region it encloses. Enclosure
// uses cardinal adjacency and a flood fill from the board edge, treating
// the capturer's territory as the boundary; cells unreached by the fill
// are enclosed. Opponent starting territory is never claimed.
class TerritoryResolver {
public:
    void applyCapture(territorygame::domain::GameState& state, territorygame::domain::PlayerId capturerId);

private:
    std::vector<territorygame::api::GridPosition> findEnclosedCells(
        territorygame::domain::Board& board, territorygame::domain::PlayerId capturerId);

    // The flood fill can enclose (and therefore claim) an opponent's
    // starting platform along with everything else inside the loop; restore
    // it afterward so a player's home can never be captured away entirely.
    void restoreOpponentStartingTerritories(
        territorygame::domain::GameState& state, territorygame::domain::PlayerId capturerId);

    void seedIfOutsideTerritory(
        territorygame::domain::Board& board, territorygame::domain::PlayerId capturerId,
        territorygame::api::GridPosition position, std::vector<std::vector<bool>>& reachedFromEdge,
        std::deque<territorygame::api::GridPosition>& frontier);

    std::vector<territorygame::api::GridPosition> cardinalNeighbors(
        territorygame::api::GridPosition position, int width, int height);
};

} // namespace territorygame::rules
