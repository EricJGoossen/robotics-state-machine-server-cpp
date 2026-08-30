#include "candidate/examples/RandomStateMachine.hpp"

#include "territorygame/helpers/MovementUtils.hpp"

namespace candidate::examples {

using territorygame::api::Direction;
using territorygame::api::GameApi;
using territorygame::helpers::MovementUtils;

RandomStateMachine::RandomStateMachine() : random_(7) {}

void RandomStateMachine::takeTurn(GameApi& game) {
    auto validDirections = MovementUtils::validDirections(game);

    Direction choice;
    if (validDirections.empty()) {
        choice = MovementUtils::randomDirection(random_);
    } else {
        std::uniform_int_distribution<size_t> dist(0, validDirections.size() - 1);
        choice = validDirections[dist(random_)];
    }

    game.move(choice);
}

} // namespace candidate::examples
