#pragma once

#include <vector>

#include "territorygame/api/GridPosition.hpp"
#include "territorygame/domain/Agent.hpp"
#include "territorygame/domain/PlayerId.hpp"

namespace territorygame::domain {

// A match participant: identity, moving agent, and configured starting territory.
class Player {
public:
    Player(PlayerId id, Agent agent, std::vector<territorygame::api::GridPosition> startingTerritory);

    PlayerId getId() const;
    Agent& getAgent();
    const Agent& getAgent() const;
    const std::vector<territorygame::api::GridPosition>& getStartingTerritory() const;

private:
    PlayerId id_;
    Agent agent_;
    std::vector<territorygame::api::GridPosition> startingTerritory_;
};

} // namespace territorygame::domain
