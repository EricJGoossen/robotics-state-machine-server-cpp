#pragma once

#include <random>

#include "territorygame/api/AgentController.hpp"
#include "territorygame/api/GameApi.hpp"

namespace candidate::examples {

// Simplest baseline controller: picks uniformly among directions that are
// mechanically valid (in bounds, not onto the opponent's agent). Does not
// avoid its own trail, so it can legitimately kill itself. Not really a
// state machine -- no persistent decision state, just a random pick each
// turn -- the name just matches the other examples for consistency.
class RandomStateMachine final : public territorygame::api::AgentController {
public:
    RandomStateMachine();

    void takeTurn(territorygame::api::GameApi& game) override;

private:
    std::mt19937_64 random_;
};

} // namespace candidate::examples
