#pragma once

#include <vector>

#include "territorygame/api/Direction.hpp"
#include "territorygame/api/GridPosition.hpp"
#include "territorygame/api/MoveResult.hpp"
#include "territorygame/api/VisibleCell.hpp"

namespace territorygame::api {

/**
 * Player-scoped view of the game a controller uses to observe state and
 * move. Implementations must never leak authoritative backend types or
 * mutable references.
 */
class GameApi {
public:
    virtual ~GameApi() = default;

    virtual GridPosition getAgentPosition() const = 0;
    virtual GridPosition getRespawnPosition() const = 0;
    virtual int getOwnedTerritoryCellCount() const = 0;
    virtual int getOpponentTerritoryCellCount() const = 0;
    virtual int getRemainingTurns() const = 0;

    // The player's current active trail, ordered oldest to newest.
    virtual std::vector<GridPosition> getActiveTrail() const = 0;

    // Cells visible around the agent, indexed [row][column] i.e. [y][x] within the window.
    virtual std::vector<std::vector<VisibleCell>> getVisibleGrid() const = 0;

    virtual int getBoardWidth() const = 0;
    virtual int getBoardHeight() const = 0;

    virtual MoveResult move(Direction direction) = 0;
};

} // namespace territorygame::api
