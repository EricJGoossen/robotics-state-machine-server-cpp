#include "territorygame/rules/MoveResolver.hpp"

#include "territorygame/helpers/MovementUtils.hpp"

namespace territorygame::rules {

using territorygame::api::Direction;
using territorygame::api::GridPosition;
using territorygame::api::MoveResult;
using territorygame::domain::Agent;
using territorygame::domain::Board;
using territorygame::domain::GameState;
using territorygame::domain::Player;
using territorygame::domain::PlayerId;
using territorygame::helpers::MovementUtils;

MoveResolver::MoveResolver(RespawnService respawnService, TerritoryResolver territoryResolver)
    : respawnService_(respawnService), territoryResolver_(territoryResolver) {}

MoveResult MoveResolver::resolve(GameState& state, PlayerId moverId, Direction direction) {
    Board& board = state.getBoard();
    Player& mover = state.getPlayer(moverId);
    Player& opponent = state.getOpponent(moverId);
    Agent& moverAgent = mover.getAgent();

    GridPosition destination = MovementUtils::nextPosition(moverAgent.getPosition(), direction);
    if (!board.isWithinBounds(destination)) {
        return MoveResult::INVALID;
    }
    if (destination == opponent.getAgent().getPosition()) {
        return MoveResult::INVALID;
    }

    auto trailOwnerAtDestination = board.trailOwnerAt(destination);
    if (trailOwnerAtDestination == moverId) {
        respawnService_.respawn(state, moverId);
        state.incrementDeathCount(moverId);
        state.decrementRemainingTurns(moverId);
        return MoveResult::DIED;
    }

    // Move the mover onto its destination before respawning a killed
    // opponent, so RespawnService's occupancy check sees where the mover
    // actually ends up rather than where it moved from. Otherwise, if the
    // mover is stepping onto the opponent's own respawn point, the
    // opponent could respawn there and the mover would then move onto the
    // same cell.
    moverAgent.setPosition(destination);

    if (trailOwnerAtDestination == opponent.getId()) {
        respawnService_.respawn(state, opponent.getId());
        state.incrementKillCount(moverId);
        state.incrementDeathCount(opponent.getId());
    }

    auto territoryOwnerAtDestination = board.territoryOwnerAt(destination);
    if (territoryOwnerAtDestination == moverId && !moverAgent.isTrailEmpty()) {
        territoryResolver_.applyCapture(state, moverId);
        state.decrementRemainingTurns(moverId);
        return MoveResult::CAPTURED;
    }

    if (territoryOwnerAtDestination != moverId) {
        board.setTrailOwner(destination, moverId);
        moverAgent.appendTrail(destination);
    }
    state.decrementRemainingTurns(moverId);
    return MoveResult::MOVED;
}

} // namespace territorygame::rules
