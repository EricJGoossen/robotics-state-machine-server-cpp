#pragma once

// Replace this stub with your own implementation. See CANDIDATE_GUIDE.md
// for the game rules, the GameApi surface, and the helpers you're given.

#include "territorygame/api/AgentController.hpp"
#include "territorygame/api/Direction.hpp"
#include "territorygame/api/GameApi.hpp"
#include "territorygame/helpers/MovementUtils.hpp"

namespace candidate {

class CandidateController final : public territorygame::api::AgentController {
public:
    void takeTurn(territorygame::api::GameApi& game) override {
        if (territorygame::helpers::MovementUtils::isValidMove(game, territorygame::api::Direction::EAST)) {
            game.move(territorygame::api::Direction::EAST);
        } else {
            game.move(territorygame::api::Direction::NORTH);
        }
    }
};

} // namespace candidate
