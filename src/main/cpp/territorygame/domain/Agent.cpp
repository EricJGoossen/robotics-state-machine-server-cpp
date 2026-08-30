#include "territorygame/domain/Agent.hpp"

namespace territorygame::domain {

using territorygame::api::GridPosition;

Agent::Agent(GridPosition position, GridPosition respawnPosition)
    : respawnPosition_(respawnPosition), position_(position) {}

GridPosition Agent::getPosition() const { return position_; }

void Agent::setPosition(GridPosition position) { position_ = position; }

GridPosition Agent::getRespawnPosition() const { return respawnPosition_; }

std::vector<GridPosition> Agent::getActiveTrail() const { return activeTrail_; }

bool Agent::isTrailEmpty() const { return activeTrail_.empty(); }

void Agent::appendTrail(GridPosition cell) { activeTrail_.push_back(cell); }

void Agent::clearTrail() { activeTrail_.clear(); }

} // namespace territorygame::domain
