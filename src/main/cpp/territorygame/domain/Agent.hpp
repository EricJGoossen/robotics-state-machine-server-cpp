#pragma once

#include <vector>

#include "territorygame/api/GridPosition.hpp"

namespace territorygame::domain {

// The moving piece a player controls: its position, configured respawn
// point, and ordered active trail. Holds no decision-making strategy.
//
// Mutators are public for use by the rules/engine layers. The
// candidate-facing boundary is preserved not by C++ access control but by
// never handing an Agent reference to candidate code or the GUI.
class Agent {
public:
    Agent(territorygame::api::GridPosition position, territorygame::api::GridPosition respawnPosition);

    territorygame::api::GridPosition getPosition() const;
    void setPosition(territorygame::api::GridPosition position);

    territorygame::api::GridPosition getRespawnPosition() const;

    std::vector<territorygame::api::GridPosition> getActiveTrail() const;
    bool isTrailEmpty() const;
    void appendTrail(territorygame::api::GridPosition cell);
    void clearTrail();

private:
    territorygame::api::GridPosition respawnPosition_;
    territorygame::api::GridPosition position_;
    std::vector<territorygame::api::GridPosition> activeTrail_;
};

} // namespace territorygame::domain
