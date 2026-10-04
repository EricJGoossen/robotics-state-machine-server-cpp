#pragma once

#include "territorygame/api/GridPosition.hpp"
#include "territorygame/domain/GameState.hpp"
#include "territorygame/domain/PlayerId.hpp"

namespace territorygame::rules {

// Owns the complete death/reset operation for a player: clearing their
// trail and territory, restoring starting territory, and repositioning
// their agent.
class RespawnService {
public:
    void respawn(territorygame::domain::GameState& state, territorygame::domain::PlayerId playerId);

private:
    territorygame::api::GridPosition choosePosition(
        territorygame::domain::GameState& state, territorygame::domain::PlayerId playerId);
};

} // namespace territorygame::rules
