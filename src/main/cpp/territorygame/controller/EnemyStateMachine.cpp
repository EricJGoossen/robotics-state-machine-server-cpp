#include "territorygame/controller/EnemyStateMachine.hpp"

#include <limits>
#include <sstream>

#include "territorygame/helpers/MovementUtils.hpp"

namespace territorygame::controller {

using namespace territorygame::api;
using territorygame::helpers::MovementUtils;

EnemyStateMachine::EnemyStateMachine() : EnemyStateMachine(std::random_device{}()) {}

EnemyStateMachine::EnemyStateMachine(int64_t seed) : random_(static_cast<uint64_t>(seed)) {}

void EnemyStateMachine::takeTurn(GameApi& game) {
    // Even with different seeds, two fresh instances facing a symmetric
    // starting position can still tie on every heuristic and open in the
    // same relative direction. Forcing a genuinely random opening move
    // (not just WANDERING's near-best random pick) breaks that up front.
    if (firstMove_) {
        firstMove_ = false;
        currentState_ = State::WANDERING;
        game.move(pickUniformlyRandom(game));
        return;
    }
    State previousState = currentState_;
    currentState_ = decideState(game, previousState);
    Direction direction = chooseDirection(game, currentState_);
    game.move(direction);
}

namespace {
std::string directionName(std::optional<Direction> direction) {
    if (!direction.has_value()) {
        return "null";
    }
    switch (*direction) {
        case Direction::NORTH: return "NORTH";
        case Direction::SOUTH: return "SOUTH";
        case Direction::EAST: return "EAST";
        case Direction::WEST: return "WEST";
    }
    return "null";
}
} // namespace

std::optional<std::string> EnemyStateMachine::getDebugState() const {
    std::string stateName;
    switch (currentState_) {
        case State::DEFENSIVE: stateName = "DEFENSIVE"; break;
        case State::RECEDING: stateName = "RECEDING"; break;
        case State::AGGRESSIVE: stateName = "AGGRESSIVE"; break;
        case State::EXPANDING: stateName = "EXPANDING"; break;
        case State::WANDERING: stateName = "WANDERING"; break;
    }
    std::ostringstream oss;
    oss << stateName << " " << directionName(direction_) << ", bestOpenNeighborCount: " << bestOpenNeighborCount_;
    return oss.str();
}

// ---- State selection ----------------------------------------------

EnemyStateMachine::State EnemyStateMachine::decideState(GameApi& game, State previousState) {
    bool territoryShrank = game.getOwnedTerritoryCellCount() < previousOwnedTerritoryCount_;
    previousOwnedTerritoryCount_ = game.getOwnedTerritoryCellCount();
    if (territoryShrank) {
        return State::DEFENSIVE;
    }
    if (shouldRecede(game)) {
        return State::RECEDING;
    }
    if (shouldBeAggressive(game)) {
        return State::AGGRESSIVE;
    }
    std::uniform_real_distribution<double> chance(0.0, 1.0);
    if (previousState != State::WANDERING && chance(random_) < RANDOM_WANDER_CHANCE) {
        return State::WANDERING;
    }
    return State::EXPANDING;
}

bool EnemyStateMachine::shouldRecede(GameApi& game) {
    auto trail = game.getActiveTrail();
    if (!trail.empty()) {
        if (static_cast<int>(trail.size()) >= MAX_TRAIL_BEFORE_RETURN) {
            return true;
        }
        if (game.getRemainingTurns() <= static_cast<int>(trail.size()) + SAFETY_TURN_BUFFER) {
            return true;
        }
        if (opponentIsThreateninglyClose(game)) {
            return true;
        }
    }
    return isEndgameWithLead(game);
}

bool EnemyStateMachine::shouldBeAggressive(GameApi& game) {
    return nearestOccupant(game, OccupantView::OPPONENT_TRAIL).has_value()
        || nearestTerritory(game, TerritoryView::OPPONENT).has_value();
}

bool EnemyStateMachine::opponentIsThreateninglyClose(GameApi& game) {
    int threatDistance = static_cast<int>(game.getVisibleGrid().size()) / 2;
    auto position = nearestOccupant(game, OccupantView::OPPONENT_AGENT);
    if (!position.has_value()) {
        return false;
    }
    return MovementUtils::manhattanDistance(game.getAgentPosition(), *position) <= threatDistance;
}

bool EnemyStateMachine::isEndgameWithLead(GameApi& game) {
    return game.getRemainingTurns() <= CONSOLIDATE_TURNS_THRESHOLD
        && game.getOwnedTerritoryCellCount() > game.getOpponentTerritoryCellCount();
}

// ---- Direction selection --------------------------------------------

Direction EnemyStateMachine::chooseDirection(GameApi& game, State state) {
    switch (state) {
        case State::DEFENSIVE: return pickDefensive(game);
        case State::RECEDING: return pickReceding(game);
        case State::AGGRESSIVE: return pickAggressive(game);
        case State::EXPANDING: return pickExpanding(game);
        case State::WANDERING: return pickWandering(game);
    }
    return fallback();
}

// Chases the opponent's visible trail to stop an in-progress capture cold; otherwise falls back to heading home.
Direction EnemyStateMachine::pickDefensive(GameApi& game) {
    auto hunted = huntOpponentTrail(game);
    return hunted.has_value() ? *hunted : pickReceding(game);
}

// Deterministically pushes toward whichever safe direction opens onto the most free space.
Direction EnemyStateMachine::pickExpanding(GameApi& game) {
    Direction best = chooseBest(safeDirections(game), [this, &game](Direction d) { return -openNeighborCount(game, d); });
    direction_ = best;
    bestOpenNeighborCount_ = openNeighborCount(game, best);
    return best;
}

// Stays inside our own territory if any safe move lands there (zero trail risk); otherwise heads for the nearest of it.
Direction EnemyStateMachine::pickReceding(GameApi& game) {
    auto safe = safeDirections(game);
    std::vector<Direction> withinTerritory;
    for (Direction d : safe) {
        if (territoryAt(game, destination(game, d)) == TerritoryView::SELF) {
            withinTerritory.push_back(d);
        }
    }
    if (!withinTerritory.empty()) {
        return chooseBest(withinTerritory, [this, &game](Direction d) { return -openNeighborCount(game, d); });
    }
    auto visible = nearestTerritory(game, TerritoryView::SELF);
    GridPosition target = visible.has_value() ? *visible : game.getRespawnPosition();
    return chooseBest(safe, [&game, target](Direction d) {
        return MovementUtils::manhattanDistance(MovementUtils::nextPosition(game.getAgentPosition(), d), target);
    });
}

// Chases the opponent's trail for a kill if one's visible; otherwise cuts toward their territory to steal it on capture.
Direction EnemyStateMachine::pickAggressive(GameApi& game) {
    auto hunted = huntOpponentTrail(game);
    if (hunted.has_value()) {
        return *hunted;
    }
    auto visible = nearestTerritory(game, TerritoryView::OPPONENT);
    GridPosition target = visible.has_value() ? *visible : game.getAgentPosition();
    return chooseBest(safeDirections(game), [&game, target](Direction d) {
        return MovementUtils::manhattanDistance(MovementUtils::nextPosition(game.getAgentPosition(), d), target);
    });
}

// Shared by DEFENSIVE and AGGRESSIVE: a direction that closes on the opponent's visible trail, if one is visible at all.
std::optional<Direction> EnemyStateMachine::huntOpponentTrail(GameApi& game) {
    auto target = nearestOccupant(game, OccupantView::OPPONENT_TRAIL);
    if (!target.has_value()) {
        return std::nullopt;
    }
    GridPosition t = *target;
    return chooseBest(huntableDirections(game), [&game, t](Direction d) {
        return MovementUtils::manhattanDistance(MovementUtils::nextPosition(game.getAgentPosition(), d), t);
    });
}

// Picks at random among the directions whose open-space score is close to the best, so it's never fully predictable.
Direction EnemyStateMachine::pickWandering(GameApi& game) {
    auto candidates = safeDirections(game);
    if (candidates.empty()) {
        return fallback();
    }
    int bestScore = std::numeric_limits<int>::min();
    for (Direction d : candidates) {
        bestScore = std::max(bestScore, openNeighborCount(game, d));
    }
    std::vector<Direction> goodEnough;
    for (Direction d : candidates) {
        if (openNeighborCount(game, d) >= bestScore - WANDER_OPENNESS_TOLERANCE) {
            goodEnough.push_back(d);
        }
    }
    std::uniform_int_distribution<size_t> dist(0, goodEnough.size() - 1);
    return goodEnough[dist(random_)];
}

// Picks uniformly among every safe direction, with no bias toward open space at all -- only used for the opening move.
Direction EnemyStateMachine::pickUniformlyRandom(GameApi& game) {
    auto candidates = safeDirections(game);
    if (candidates.empty()) {
        return fallback();
    }
    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    return candidates[dist(random_)];
}

Direction EnemyStateMachine::chooseBest(
    const std::vector<Direction>& candidates, const std::function<int(Direction)>& key) {
    if (candidates.empty()) {
        return fallback();
    }
    Direction best = candidates.front();
    int bestKey = key(best);
    for (size_t i = 1; i < candidates.size(); i++) {
        int k = key(candidates[i]);
        if (k < bestKey) {
            bestKey = k;
            best = candidates[i];
        }
    }
    return best;
}

// ---- Board reading -----------------------------------------------------

// Directions that are in bounds and land on neither trail nor the opponent's agent.
std::vector<Direction> EnemyStateMachine::safeDirections(GameApi& game) {
    std::vector<Direction> result;
    for (Direction direction : DIRECTIONS) {
        if (!MovementUtils::isValidBoardMove(
                game.getAgentPosition(), direction, game.getBoardWidth(), game.getBoardHeight())) {
            continue;
        }
        auto occupant = occupantAt(game, destination(game, direction));
        if (occupant != OccupantView::SELF_TRAIL && occupant != OccupantView::OPPONENT_TRAIL
                && occupant != OccupantView::OPPONENT_AGENT) {
            result.push_back(direction);
        }
    }
    return result;
}

// Like safeDirections, but allows stepping onto the opponent's trail -- that's the point of hunting.
std::vector<Direction> EnemyStateMachine::huntableDirections(GameApi& game) {
    std::vector<Direction> result;
    for (Direction direction : DIRECTIONS) {
        if (!MovementUtils::isValidBoardMove(
                game.getAgentPosition(), direction, game.getBoardWidth(), game.getBoardHeight())) {
            continue;
        }
        auto occupant = occupantAt(game, destination(game, direction));
        if (occupant != OccupantView::SELF_TRAIL && occupant != OccupantView::OPPONENT_AGENT) {
            result.push_back(direction);
        }
    }
    return result;
}

// Count of on-board empty unowned cells cardinally adjacent to the destination. Off-board neighbors are not open.
int EnemyStateMachine::openNeighborCount(GameApi& game, Direction direction) {
    GridPosition dest = destination(game, direction);
    int count = 0;
    for (Direction neighborDirection : DIRECTIONS) {
        GridPosition neighbor = MovementUtils::nextPosition(dest, neighborDirection);
        if (isOpen(cellAt(game, neighbor))) {
            count++;
        }
    }
    return count;
}

std::optional<GridPosition> EnemyStateMachine::nearestOccupant(GameApi& game, OccupantView occupant) {
    return nearestVisible(game, [occupant](const VisibleCell& cell) { return cell.occupant == occupant; });
}

std::optional<GridPosition> EnemyStateMachine::nearestTerritory(GameApi& game, TerritoryView territory) {
    return nearestVisible(game, [territory](const VisibleCell& cell) { return cell.territory == territory; });
}

std::optional<GridPosition> EnemyStateMachine::nearestVisible(
    GameApi& game, const std::function<bool(const VisibleCell&)>& match) {
    GridPosition from = game.getAgentPosition();
    std::optional<GridPosition> best;
    int bestDistance = std::numeric_limits<int>::max();
    for (const auto& row : game.getVisibleGrid()) {
        for (const auto& cell : row) {
            if (match(cell)) {
                int distance = MovementUtils::manhattanDistance(from, cell.position);
                if (distance < bestDistance) {
                    bestDistance = distance;
                    best = cell.position;
                }
            }
        }
    }
    return best;
}

GridPosition EnemyStateMachine::destination(GameApi& game, Direction direction) {
    return MovementUtils::nextPosition(game.getAgentPosition(), direction);
}

// Absent when position is off the board (a corner or edge), not an open cell.
std::optional<VisibleCell> EnemyStateMachine::cellAt(GameApi& game, GridPosition position) {
    if (!MovementUtils::isWithinBoard(position, game.getBoardWidth(), game.getBoardHeight())) {
        return std::nullopt;
    }
    auto found = MovementUtils::findCell(game.getVisibleGrid(), position);
    if (found.has_value()) {
        return found;
    }
    return VisibleCell{position, OccupantView::EMPTY, TerritoryView::UNOWNED};
}

std::optional<OccupantView> EnemyStateMachine::occupantAt(GameApi& game, GridPosition position) {
    auto cell = cellAt(game, position);
    return cell.has_value() ? std::optional<OccupantView>(cell->occupant) : std::nullopt;
}

std::optional<TerritoryView> EnemyStateMachine::territoryAt(GameApi& game, GridPosition position) {
    auto cell = cellAt(game, position);
    return cell.has_value() ? std::optional<TerritoryView>(cell->territory) : std::nullopt;
}

bool EnemyStateMachine::isOpen(const std::optional<VisibleCell>& cell) {
    return cell.has_value() && cell->occupant == OccupantView::EMPTY && cell->territory == TerritoryView::UNOWNED;
}

Direction EnemyStateMachine::fallback() {
    return MovementUtils::randomDirection(random_);
}

} // namespace territorygame::controller
