#include <vector>

#include "testing/TestRunner.hpp"

#include "territorygame/TestGames.hpp"
#include "territorygame/rules/MoveResolver.hpp"
#include "territorygame/rules/RespawnService.hpp"
#include "territorygame/rules/TerritoryResolver.hpp"

using namespace territorygame::api;
using territorygame::domain::GameState;
using territorygame::domain::PlayerId;
using territorygame::rules::MoveResolver;
using territorygame::rules::RespawnService;
using territorygame::rules::TerritoryResolver;

namespace {
const PlayerId player0{0};
const PlayerId player1{1};

MoveResolver makeResolver() { return MoveResolver(RespawnService(), TerritoryResolver()); }
} // namespace

TEST(MoveResolverTest, validMoveOutsideTerritoryStartsATrailAndConsumesATurn) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, {GridPosition{1, 1}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver resolver = makeResolver();

    MoveResult result = resolver.resolve(state, player0, Direction::EAST);

    EXPECT_EQ(result, MoveResult::MOVED);
    EXPECT_EQ(state.getPlayer(player0).getAgent().getPosition(), (GridPosition{2, 1}));
    EXPECT_EQ(state.getPlayer(player0).getAgent().getActiveTrail(), (std::vector<GridPosition>{GridPosition{2, 1}}));
    EXPECT_EQ(state.getRemainingTurns(player0), 9);
}

TEST(MoveResolverTest, moveOffTheBoardIsInvalid) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{0, 0}, {GridPosition{0, 0}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver resolver = makeResolver();

    MoveResult result = resolver.resolve(state, player0, Direction::NORTH);

    EXPECT_EQ(result, MoveResult::INVALID);
}

TEST(MoveResolverTest, moveOntoOpponentAgentIsInvalid) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, {GridPosition{1, 1}}, GridPosition{2, 1}, {GridPosition{2, 1}}, 10);
    MoveResolver resolver = makeResolver();

    MoveResult result = resolver.resolve(state, player0, Direction::EAST);

    EXPECT_EQ(result, MoveResult::INVALID);
}

TEST(MoveResolverTest, invalidMoveLeavesPositionAndTrailUnchanged) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{0, 0}, {GridPosition{0, 0}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver resolver = makeResolver();

    resolver.resolve(state, player0, Direction::NORTH);

    EXPECT_EQ(state.getPlayer(player0).getAgent().getPosition(), (GridPosition{0, 0}));
    EXPECT_TRUE(state.getPlayer(player0).getAgent().getActiveTrail().empty());
}

TEST(MoveResolverTest, invalidMoveDoesNotDecrementTurns) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{0, 0}, {GridPosition{0, 0}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver resolver = makeResolver();

    resolver.resolve(state, player0, Direction::NORTH);

    EXPECT_EQ(state.getRemainingTurns(player0), 10);
}

TEST(MoveResolverTest, secondMoveOutsideTerritoryExtendsTheTrail) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, {GridPosition{1, 1}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver resolver = makeResolver();

    resolver.resolve(state, player0, Direction::EAST);
    resolver.resolve(state, player0, Direction::EAST);

    std::vector<GridPosition> expected{GridPosition{2, 1}, GridPosition{3, 1}};
    EXPECT_EQ(state.getPlayer(player0).getAgent().getActiveTrail(), expected);
}

TEST(MoveResolverTest, movingWithinOwnTerritoryWithNoActiveTrailIsJustAMove) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, {GridPosition{1, 1}, GridPosition{2, 1}}, GridPosition{6, 6},
        {GridPosition{6, 6}}, 10);
    MoveResolver resolver = makeResolver();

    MoveResult result = resolver.resolve(state, player0, Direction::EAST);

    EXPECT_EQ(result, MoveResult::MOVED);
    EXPECT_TRUE(state.getPlayer(player0).getAgent().getActiveTrail().empty());
}

TEST(MoveResolverTest, returningToOwnTerritoryWithANonEmptyTrailCloses) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{2, 2}, {GridPosition{2, 2}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver resolver = makeResolver();

    resolver.resolve(state, player0, Direction::EAST); // -> (3,2), trail=[(3,2)]
    MoveResult result = resolver.resolve(state, player0, Direction::WEST); // back to (2,2)

    EXPECT_EQ(result, MoveResult::CAPTURED);
    EXPECT_TRUE(state.getPlayer(player0).getAgent().getActiveTrail().empty());
}

TEST(MoveResolverTest, movingOntoOwnActiveTrailKillsTheMover) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{2, 2}, {GridPosition{2, 2}}, GridPosition{7, 7}, {GridPosition{7, 7}}, 10);
    MoveResolver resolver = makeResolver();

    resolver.resolve(state, player0, Direction::EAST);  // (3,2) trail
    resolver.resolve(state, player0, Direction::EAST);  // (4,2) trail
    resolver.resolve(state, player0, Direction::SOUTH); // (4,3) trail
    resolver.resolve(state, player0, Direction::WEST);  // (3,3) trail
    MoveResult result = resolver.resolve(state, player0, Direction::NORTH); // -> (3,2), own trail

    EXPECT_EQ(result, MoveResult::DIED);
    EXPECT_EQ(state.getPlayer(player0).getAgent().getPosition(), (GridPosition{2, 2}));
    EXPECT_TRUE(state.getPlayer(player0).getAgent().getActiveTrail().empty());
    EXPECT_EQ(state.getBoard().territoryCount(player0), 1);
}

TEST(MoveResolverTest, crossingOpponentTrailKillsThemAndMoverSurvivesAndContinues) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{7, 7}, {GridPosition{7, 7}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver resolver = makeResolver();

    // Player1 lays a trail cell at (7,6) then steps away to (7,5).
    resolver.resolve(state, player1, Direction::EAST);  // (6,6)->(7,6) trail
    resolver.resolve(state, player1, Direction::NORTH); // (7,6)->(7,5) trail

    // Player0 steps onto (7,6): opponent's OLD trail cell laid two moves ago,
    // not currently occupied by player1's agent (which is now at (7,5)).
    MoveResult result = resolver.resolve(state, player0, Direction::NORTH); // (7,7)->(7,6)

    EXPECT_EQ(result, MoveResult::MOVED);
    EXPECT_EQ(state.getPlayer(player0).getAgent().getPosition(), (GridPosition{7, 6}));
    // Player1 died and respawned back at its configured respawn position.
    EXPECT_EQ(state.getPlayer(player1).getAgent().getPosition(), (GridPosition{6, 6}));
    EXPECT_TRUE(state.getPlayer(player1).getAgent().getActiveTrail().empty());
    // The cell player0 stepped onto is now player0's own trail, not player1's.
    EXPECT_EQ(state.getBoard().trailOwnerAt(GridPosition{7, 6}), player0);
}

TEST(MoveResolverTest, crossingOpponentTrailIncrementsTheMoverKillCount) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{7, 7}, {GridPosition{7, 7}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver resolver = makeResolver();
    resolver.resolve(state, player1, Direction::EAST);  // (6,6)->(7,6) trail
    resolver.resolve(state, player1, Direction::NORTH); // (7,6)->(7,5) trail

    resolver.resolve(state, player0, Direction::NORTH); // (7,7)->(7,6), crosses player1's trail

    EXPECT_EQ(state.getKillCount(player0), 1);
    EXPECT_EQ(state.getKillCount(player1), 0);
}

TEST(MoveResolverTest, closingALoopOnACellThatKillsTheOpponentPreservesTheirStartingTerritory) {
    // Player0 returns home (closing a loop) onto a cell that also has
    // player1's trail, so both a capture and a kill resolve in one move.
    // Player0's existing territory already rings player1's start, so the
    // capture flood-fill would otherwise paint over the start platform
    // that respawn just restored.
    GridPosition p1Start{4, 4};
    std::vector<GridPosition> p0Territory{
        GridPosition{1, 1},
        GridPosition{3, 3}, GridPosition{4, 3}, GridPosition{5, 3},
        GridPosition{3, 4}, GridPosition{5, 4},
        GridPosition{3, 5}, GridPosition{4, 5}, GridPosition{5, 5}};
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, p0Territory, p1Start, {p1Start}, 10);
    MoveResolver resolver = makeResolver();

    state.getPlayer(player0).getAgent().setPosition(GridPosition{1, 2});
    state.getBoard().setTrailOwner(GridPosition{1, 2}, player0);
    state.getPlayer(player0).getAgent().appendTrail(GridPosition{1, 2});

    state.getPlayer(player1).getAgent().setPosition(GridPosition{1, 3});
    state.getBoard().setTrailOwner(GridPosition{1, 1}, player1);
    state.getPlayer(player1).getAgent().appendTrail(GridPosition{1, 1});

    MoveResult result = resolver.resolve(state, player0, Direction::NORTH);

    EXPECT_EQ(result, MoveResult::CAPTURED);
    EXPECT_EQ(state.getKillCount(player0), 1);
    EXPECT_EQ(state.getPlayer(player1).getAgent().getPosition(), p1Start);
    EXPECT_EQ(state.getBoard().territoryOwnerAt(p1Start), player1);
    EXPECT_EQ(state.getBoard().territoryCount(player1), 1);
}

TEST(MoveResolverTest, movingOntoOpponentsTrailAtItsOwnRespawnPointDoesNotStackAgents) {
    // Regression test for the bug where killing an opponent by stepping
    // onto a trail cell that happens to sit on the opponent's own
    // respawn point could respawn the opponent onto the mover's
    // destination, stacking both agents on one cell.
    GridPosition opponentRespawn{5, 5};
    std::vector<GridPosition> opponentStartingTerritory{
        GridPosition{4, 4}, GridPosition{5, 4}, GridPosition{6, 4},
        GridPosition{4, 5}, GridPosition{5, 5}, GridPosition{6, 5},
        GridPosition{4, 6}, GridPosition{5, 6}, GridPosition{6, 6}};
    GameState state = territorygame::test::twoPlayerState(
        10, 10, GridPosition{4, 8}, {GridPosition{4, 8}}, opponentRespawn, opponentStartingTerritory, 10);
    MoveResolver resolver = makeResolver();

    // Opponent's territory has since shifted away from its respawn
    // point, leaving a trail cell sitting exactly on it.
    state.getPlayer(player1).getAgent().setPosition(GridPosition{9, 9});
    state.getBoard().setTrailOwner(opponentRespawn, player1);
    state.getPlayer(player1).getAgent().appendTrail(opponentRespawn);
    state.getPlayer(player0).getAgent().setPosition(GridPosition{4, 5});

    MoveResult result = resolver.resolve(state, player0, Direction::EAST); // (4,5) -> (5,5)

    EXPECT_EQ(result, MoveResult::MOVED);
    GridPosition moverPosition = state.getPlayer(player0).getAgent().getPosition();
    GridPosition opponentNewPosition = state.getPlayer(player1).getAgent().getPosition();
    EXPECT_EQ(moverPosition, opponentRespawn);
    EXPECT_NE(moverPosition, opponentNewPosition);
    EXPECT_EQ(state.getKillCount(player0), 1);
}
