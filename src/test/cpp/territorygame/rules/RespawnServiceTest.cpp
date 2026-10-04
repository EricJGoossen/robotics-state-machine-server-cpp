#include <vector>

#include "testing/TestRunner.hpp"

#include "territorygame/TestGames.hpp"
#include "territorygame/rules/RespawnService.hpp"

using namespace territorygame::api;
using territorygame::domain::GameState;
using territorygame::domain::PlayerId;
using territorygame::rules::RespawnService;

namespace {
const PlayerId player0{0};
const PlayerId player1{1};

const GridPosition RESPAWN{5, 5};
const std::vector<GridPosition> STARTING_TERRITORY{
    GridPosition{4, 4}, GridPosition{5, 4}, GridPosition{6, 4},
    GridPosition{4, 5}, GridPosition{5, 5}, GridPosition{6, 5},
    GridPosition{4, 6}, GridPosition{5, 6}, GridPosition{6, 6}};

GameState buildState(GridPosition opponentPosition) {
    return territorygame::test::twoPlayerState(
        20, 20, RESPAWN, STARTING_TERRITORY, opponentPosition, {opponentPosition}, 10);
}

} // namespace

TEST(RespawnServiceTest, deathClearsTerritoryCapturedBeyondStartingTerritory) {
    GameState state = buildState(GridPosition{15, 15});
    GridPosition capturedElsewhere{0, 0};
    state.getBoard().setTerritoryOwner(capturedElsewhere, player0);
    RespawnService respawnService;

    respawnService.respawn(state, player0);

    EXPECT_FALSE(state.getBoard().territoryOwnerAt(capturedElsewhere).has_value());
}

TEST(RespawnServiceTest, deathRestoresExactlyTheStartingTerritory) {
    GameState state = buildState(GridPosition{15, 15});
    RespawnService respawnService;

    respawnService.respawn(state, player0);

    for (const auto& cell : STARTING_TERRITORY) {
        EXPECT_EQ(state.getBoard().territoryOwnerAt(cell), player0);
    }
    EXPECT_EQ(state.getBoard().territoryCount(player0), static_cast<int>(STARTING_TERRITORY.size()));
}

TEST(RespawnServiceTest, respawnPlacesAgentAtConfiguredPositionWhenFree) {
    GameState state = buildState(GridPosition{15, 15});
    RespawnService respawnService;

    respawnService.respawn(state, player0);

    EXPECT_EQ(state.getPlayer(player0).getAgent().getPosition(), RESPAWN);
}

TEST(RespawnServiceTest, respawnFallsBackToNearestUnoccupiedStartingCellWhenRespawnPositionIsOccupied) {
    // Opponent sits exactly on player0's respawn cell.
    GameState state = buildState(RESPAWN);
    RespawnService respawnService;

    respawnService.respawn(state, player0);

    // Four cells are at Manhattan distance 1 from (5,5): (5,4),(4,5),(6,5),(5,6).
    // Lowest y first breaks the tie: (5,4).
    EXPECT_EQ(state.getPlayer(player0).getAgent().getPosition(), (GridPosition{5, 4}));
}

TEST(RespawnServiceTest, activeTrailIsClearedOnDeath) {
    GameState state = buildState(GridPosition{15, 15});
    GridPosition trailCell{10, 10};
    state.getBoard().setTrailOwner(trailCell, player0);
    state.getPlayer(player0).getAgent().appendTrail(trailCell);
    RespawnService respawnService;

    respawnService.respawn(state, player0);

    EXPECT_TRUE(state.getPlayer(player0).getAgent().getActiveTrail().empty());
    EXPECT_FALSE(state.getBoard().trailOwnerAt(trailCell).has_value());
}

TEST(RespawnServiceTest, respawnFallsBackToTheWholeBoardWhenStartingTerritoryIsFullyBlocked) {
    // A single-cell starting territory with the opponent standing on it:
    // the starting-territory fallback has nowhere to go, so this must
    // fall back to the nearest free cell anywhere on the board instead
    // of throwing.
    GridPosition singleCellRespawn{5, 5};
    GameState state = territorygame::test::twoPlayerState(
        20, 20, singleCellRespawn, {singleCellRespawn}, singleCellRespawn, {singleCellRespawn}, 10);
    RespawnService respawnService;

    respawnService.respawn(state, player0);

    // Four cells are at Manhattan distance 1 from (5,5); lowest y then x breaks the tie.
    EXPECT_EQ(state.getPlayer(player0).getAgent().getPosition(), (GridPosition{5, 4}));
}

TEST(RespawnServiceTest, respawningOnePlayerDoesNotAffectTheOther) {
    GameState state = buildState(GridPosition{15, 15});
    RespawnService respawnService;

    respawnService.respawn(state, player0);

    EXPECT_EQ(state.getPlayer(player1).getAgent().getPosition(), (GridPosition{15, 15}));
    EXPECT_EQ(state.getBoard().territoryCount(player1), 1);
}
