#pragma once

#include <optional>
#include <vector>

#include "territorygame/api/GameApi.hpp"
#include "territorygame/domain/GameState.hpp"
#include "territorygame/domain/PlayerId.hpp"
#include "territorygame/rules/MoveResolver.hpp"
#include "territorygame/visibility/VisibilityService.hpp"

namespace territorygame::engine {

// Player-scoped facade over authoritative state and the move operation.
// One instance is created per player and reused for the whole match; it
// never exposes backend domain objects, and every returned collection is a
// defensive copy so controller code cannot mutate authoritative state.
class GameApiImpl : public territorygame::api::GameApi {
public:
    GameApiImpl(
        territorygame::domain::GameState& state, territorygame::domain::PlayerId owner,
        territorygame::rules::MoveResolver& moveResolver,
        territorygame::visibility::VisibilityService& visibilityService);

    territorygame::api::GridPosition getAgentPosition() const override;
    territorygame::api::GridPosition getRespawnPosition() const override;
    int getOwnedTerritoryCellCount() const override;
    int getOpponentTerritoryCellCount() const override;
    int getRemainingTurns() const override;
    std::vector<territorygame::api::GridPosition> getActiveTrail() const override;
    std::vector<std::vector<territorygame::api::VisibleCell>> getVisibleGrid() const override;
    int getBoardWidth() const override;
    int getBoardHeight() const override;
    territorygame::api::MoveResult move(territorygame::api::Direction direction) override;

    // Clears the one-successful-move-per-turn guard. Called before each turn.
    void resetForNewTurn();
    bool wasMoveMadeThisTurn() const;
    std::optional<territorygame::api::MoveResult> getLastResultThisTurn() const;

private:
    territorygame::domain::GameState& state_;
    territorygame::domain::PlayerId owner_;
    territorygame::rules::MoveResolver& moveResolver_;
    territorygame::visibility::VisibilityService& visibilityService_;

    bool hasMovedThisTurn_ = false;
    std::optional<territorygame::api::MoveResult> lastResultThisTurn_;
};

} // namespace territorygame::engine
