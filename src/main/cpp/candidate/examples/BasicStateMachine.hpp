#pragma once

#include <optional>
#include <random>
#include <string>

#include "territorygame/api/AgentController.hpp"
#include "territorygame/api/Direction.hpp"
#include "territorygame/api/GameApi.hpp"
#include "territorygame/api/GridPosition.hpp"
#include "territorygame/api/MoveResult.hpp"

namespace candidate::examples {

// Deliberately weak reference implementation. It demonstrates persistent
// controller state, state transitions, calling move(), and reacting to
// MoveResult -- not a strategy worth copying. EXPANDING still wanders
// randomly and can walk over its own trail; RETURNING avoids its own trail
// when it can, but only steps around it, not toward the nearest owned
// territory. See README.md's Tips section for ideas (finding your nearest
// territory, tracking the opponent, etc.) left undone here.
class BasicStateMachine final : public territorygame::api::AgentController {
public:
    BasicStateMachine();

    void takeTurn(territorygame::api::GameApi& game) override;
    std::optional<std::string> getDebugState() const override;

private:
    enum class State { EXPANDING, RETURNING };

    static constexpr int RETURN_TRAIL_THRESHOLD = 4;

    void updateState(territorygame::api::GameApi& game, territorygame::api::MoveResult result);
    territorygame::api::Direction pickExpandingDirection(territorygame::api::GameApi& game);
    territorygame::api::Direction pickReturningDirection(territorygame::api::GameApi& game);
    bool isSafeMove(territorygame::api::GameApi& game, territorygame::api::Direction direction);

    State state_ = State::EXPANDING;
    std::mt19937_64 random_;
};

} // namespace candidate::examples
