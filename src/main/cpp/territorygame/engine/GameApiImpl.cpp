#include "territorygame/engine/GameApiImpl.hpp"

namespace territorygame::engine {

using namespace territorygame::api;
using territorygame::domain::GameState;
using territorygame::domain::PlayerId;
using territorygame::rules::MoveResolver;
using territorygame::visibility::VisibilityService;

GameApiImpl::GameApiImpl(
    GameState& state, PlayerId owner, MoveResolver& moveResolver, VisibilityService& visibilityService)
    : state_(state), owner_(owner), moveResolver_(moveResolver), visibilityService_(visibilityService) {}

GridPosition GameApiImpl::getAgentPosition() const {
    return state_.getPlayer(owner_).getAgent().getPosition();
}

GridPosition GameApiImpl::getRespawnPosition() const {
    return state_.getPlayer(owner_).getAgent().getRespawnPosition();
}

int GameApiImpl::getOwnedTerritoryCellCount() const { return state_.getBoard().territoryCount(owner_); }

int GameApiImpl::getOpponentTerritoryCellCount() const {
    return state_.getBoard().territoryCount(state_.getOpponent(owner_).getId());
}

int GameApiImpl::getRemainingTurns() const { return state_.getRemainingTurns(owner_); }

std::vector<GridPosition> GameApiImpl::getActiveTrail() const {
    return state_.getPlayer(owner_).getAgent().getActiveTrail();
}

std::vector<std::vector<VisibleCell>> GameApiImpl::getVisibleGrid() const {
    return visibilityService_.computeVisibleGrid(state_, owner_);
}

int GameApiImpl::getBoardWidth() const { return state_.getBoard().getWidth(); }

int GameApiImpl::getBoardHeight() const { return state_.getBoard().getHeight(); }

MoveResult GameApiImpl::move(Direction direction) {
    if (hasMovedThisTurn_) {
        return MoveResult::INVALID;
    }
    MoveResult result = moveResolver_.resolve(state_, owner_, direction);
    if (result != MoveResult::INVALID) {
        hasMovedThisTurn_ = true;
        lastResultThisTurn_ = result;
    }
    return result;
}

void GameApiImpl::resetForNewTurn() {
    hasMovedThisTurn_ = false;
    lastResultThisTurn_.reset();
}

bool GameApiImpl::wasMoveMadeThisTurn() const { return hasMovedThisTurn_; }

std::optional<MoveResult> GameApiImpl::getLastResultThisTurn() const { return lastResultThisTurn_; }

} // namespace territorygame::engine
