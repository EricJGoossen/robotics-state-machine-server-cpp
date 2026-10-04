#pragma once

#include <unordered_map>

#include "territorygame/api/AgentController.hpp"
#include "territorygame/domain/GameState.hpp"
#include "territorygame/domain/PlayerId.hpp"
#include "territorygame/engine/GameApiImpl.hpp"

namespace territorygame::engine {

// Executes exactly one turn for the current active player, re-invoking
// their controller until a successful move is made, then advances to the
// next player with turns remaining.
//
// A controller that throws, or that never manages a successful move within
// maxAttemptsPerTurn attempts, forfeits just that turn rather than hanging
// the match: the reason is recorded on GameState for the caller to surface,
// and play continues with the next player.
class TurnManager {
public:
    explicit TurnManager(int maxAttemptsPerTurn);

    void executeTurn(
        territorygame::domain::GameState& state,
        std::unordered_map<territorygame::domain::PlayerId, territorygame::api::AgentController*>& controllers,
        std::unordered_map<territorygame::domain::PlayerId, GameApiImpl*>& apis);

private:
    void advanceActivePlayer(territorygame::domain::GameState& state);

    int maxAttemptsPerTurn_;
};

} // namespace territorygame::engine
