#include "territorygame/domain/Player.hpp"

#include <utility>

namespace territorygame::domain {

using territorygame::api::GridPosition;

Player::Player(PlayerId id, Agent agent, std::vector<GridPosition> startingTerritory)
    : id_(id), agent_(std::move(agent)), startingTerritory_(std::move(startingTerritory)) {}

PlayerId Player::getId() const { return id_; }

Agent& Player::getAgent() { return agent_; }

const Agent& Player::getAgent() const { return agent_; }

const std::vector<GridPosition>& Player::getStartingTerritory() const { return startingTerritory_; }

} // namespace territorygame::domain
