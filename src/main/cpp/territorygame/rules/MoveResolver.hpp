#pragma once

#include "territorygame/api/Direction.hpp"
#include "territorygame/api/MoveResult.hpp"
#include "territorygame/domain/GameState.hpp"
#include "territorygame/domain/PlayerId.hpp"
#include "territorygame/rules/RespawnService.hpp"
#include "territorygame/rules/TerritoryResolver.hpp"

namespace territorygame::rules {

// Resolves the rules of a single requested move: bounds and occupancy
// validation, trail collision (self-death, opponent-kill), trail
// extension, and trail closure/capture. Resolution order is deterministic
// and mirrors the spec's rules section exactly.
class MoveResolver {
public:
    MoveResolver(RespawnService respawnService, TerritoryResolver territoryResolver);

    territorygame::api::MoveResult resolve(
        territorygame::domain::GameState& state, territorygame::domain::PlayerId moverId,
        territorygame::api::Direction direction);

private:
    RespawnService respawnService_;
    TerritoryResolver territoryResolver_;
};

} // namespace territorygame::rules
