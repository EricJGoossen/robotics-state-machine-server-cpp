#include "candidate/examples/BasicStateMachine.hpp"

#include "territorygame/api/CellViewType.hpp"
#include "territorygame/helpers/MovementUtils.hpp"

namespace candidate::examples {

using namespace territorygame::api;
using territorygame::helpers::MovementUtils;

BasicStateMachine::BasicStateMachine() : random_(42) {}

void BasicStateMachine::takeTurn(GameApi& game) {
    Direction direction;
    if (state_ == State::EXPANDING) {
        direction = pickExpandingDirection(game);
    } else {
        direction = pickReturningDirection(game);
    }

    MoveResult result = game.move(direction);
    updateState(game, result);
}

std::optional<std::string> BasicStateMachine::getDebugState() const {
    return state_ == State::EXPANDING ? "EXPANDING" : "RETURNING";
}

void BasicStateMachine::updateState(GameApi& game, MoveResult result) {
    if (result == MoveResult::CAPTURED || result == MoveResult::DIED) {
        state_ = State::EXPANDING;
        return;
    }
    if (state_ == State::EXPANDING && game.getActiveTrail().size() >= RETURN_TRAIL_THRESHOLD) {
        state_ = State::RETURNING;
    } else if (state_ == State::RETURNING && game.getActiveTrail().empty()) {
        state_ = State::EXPANDING;
    }
}

// Wanders randomly among the mechanically safe directions.
Direction BasicStateMachine::pickExpandingDirection(GameApi& game) {
    auto safeDirections = MovementUtils::validDirections(game);
    if (safeDirections.empty()) {
        return MovementUtils::randomDirection(random_);
    }
    std::uniform_int_distribution<size_t> dist(0, safeDirections.size() - 1);
    return safeDirections[dist(random_)];
}

// Heads toward the respawn point one axis at a time; not necessarily the nearest owned cell.
Direction BasicStateMachine::pickReturningDirection(GameApi& game) {
    GridPosition position = game.getAgentPosition();
    GridPosition home = game.getRespawnPosition();

    Direction preferred;
    if (position.x < home.x) {
        preferred = Direction::EAST;
    } else if (position.x > home.x) {
        preferred = Direction::WEST;
    } else if (position.y < home.y) {
        preferred = Direction::SOUTH;
    } else if (position.y > home.y) {
        preferred = Direction::NORTH;
    } else {
        preferred = MovementUtils::randomDirection(random_);
    }

    if (isSafeMove(game, preferred)) {
        return preferred;
    }

    // Preferred direction would cross our own trail: try any other
    // direction that avoids it before accepting that risk.
    for (Direction direction : DIRECTIONS) {
        if (isSafeMove(game, direction)) {
            return direction;
        }
    }

    // Nothing avoids our own trail -- take the mechanically valid move anyway.
    if (MovementUtils::isValidMove(game, preferred)) {
        return preferred;
    }
    return MovementUtils::randomDirection(random_);
}

// Mechanically valid and not a step onto our own trail.
bool BasicStateMachine::isSafeMove(GameApi& game, Direction direction) {
    if (!MovementUtils::isValidMove(game, direction)) {
        return false;
    }
    GridPosition destination = MovementUtils::nextPosition(game.getAgentPosition(), direction);
    auto cell = MovementUtils::findCell(game.getVisibleGrid(), destination);
    if (!cell.has_value()) {
        return true;
    }
    return cell->type != CellViewType::SELF_TRAIL;
}

} // namespace candidate::examples
