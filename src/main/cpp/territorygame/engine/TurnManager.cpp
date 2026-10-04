#include "territorygame/engine/TurnManager.hpp"

#include <sstream>
#include <stdexcept>

namespace territorygame::engine {

using namespace territorygame::api;
using namespace territorygame::domain;

TurnManager::TurnManager(int maxAttemptsPerTurn) : maxAttemptsPerTurn_(maxAttemptsPerTurn) {}

void TurnManager::executeTurn(
    GameState& state,
    std::unordered_map<PlayerId, AgentController*>& controllers,
    std::unordered_map<PlayerId, GameApiImpl*>& apis) {
    PlayerId activeId = state.getActivePlayerId();
    GameApiImpl* api = apis.at(activeId);
    AgentController* controller = controllers.at(activeId);

    api->resetForNewTurn();
    state.setLastTurnError(std::nullopt);
    int attempts = 0;
    while (!api->wasMoveMadeThisTurn() && attempts < maxAttemptsPerTurn_) {
        attempts++;
        try {
            controller->takeTurn(*api);
        } catch (const std::exception& e) {
            std::ostringstream oss;
            oss << "Player " << activeId.index << "'s turn failed: " << e.what();
            state.setLastTurnError(oss.str());
            break;
        }
    }

    if (api->wasMoveMadeThisTurn()) {
        state.setLastMoveResult(api->getLastResultThisTurn());
    } else {
        state.decrementRemainingTurns(activeId);
        state.setLastMoveResult(MoveResult::INVALID);
        if (!state.getLastTurnError().has_value()) {
            std::ostringstream oss;
            oss << "Player " << activeId.index << "'s controller made no successful move after "
                << maxAttemptsPerTurn_ << " attempts";
            state.setLastTurnError(oss.str());
        }
    }
    advanceActivePlayer(state);
}

void TurnManager::advanceActivePlayer(GameState& state) {
    auto& players = state.getPlayers();
    int currentIndex = -1;
    for (size_t i = 0; i < players.size(); i++) {
        if (players[i].getId() == state.getActivePlayerId()) {
            currentIndex = static_cast<int>(i);
            break;
        }
    }
    if (currentIndex < 0) {
        throw std::runtime_error("Active player not found");
    }
    for (size_t offset = 1; offset <= players.size(); offset++) {
        PlayerId candidate = players[(static_cast<size_t>(currentIndex) + offset) % players.size()].getId();
        if (state.getRemainingTurns(candidate) > 0) {
            state.setActivePlayerId(candidate);
            return;
        }
    }
    // No player has turns remaining; leave activePlayerId as-is, the
    // engine's run loop stops once state.isGameOver() is true.
}

} // namespace territorygame::engine
