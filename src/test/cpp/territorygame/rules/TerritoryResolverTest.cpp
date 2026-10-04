// Builds a trail forming the perimeter of a 3x3 block around (2,2)-(4,4),
// with the center cell (3,3) as the only cell the flood fill should find
// enclosed, then calls applyCapture directly to check its effect on the board.

#include <vector>

#include "testing/TestRunner.hpp"

#include "territorygame/TestGames.hpp"
#include "territorygame/rules/TerritoryResolver.hpp"

using namespace territorygame::api;
using territorygame::domain::GameState;
using territorygame::domain::PlayerId;
using territorygame::rules::TerritoryResolver;

namespace {
const PlayerId capturer{0};
const PlayerId opponent{1};

const std::vector<GridPosition> PERIMETER{
    GridPosition{3, 2}, GridPosition{4, 2},
    GridPosition{4, 3}, GridPosition{4, 4},
    GridPosition{3, 4}, GridPosition{2, 4}, GridPosition{2, 3}};
const GridPosition HOME{2, 2};
const GridPosition ENCLOSED{3, 3};

GameState buildStateWithPendingTrail() {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, HOME, {HOME}, GridPosition{7, 7}, {GridPosition{7, 7}}, 10);
    for (const auto& cell : PERIMETER) {
        state.getBoard().setTrailOwner(cell, capturer);
        state.getPlayer(capturer).getAgent().appendTrail(cell);
    }
    return state;
}

} // namespace

TEST(TerritoryResolverTest, simpleRectangularCaptureClaimsTrailAndEnclosedCells) {
    GameState state = buildStateWithPendingTrail();
    TerritoryResolver resolver;

    resolver.applyCapture(state, capturer);

    for (const auto& cell : PERIMETER) {
        EXPECT_EQ(state.getBoard().territoryOwnerAt(cell), capturer);
    }
    EXPECT_EQ(state.getBoard().territoryOwnerAt(ENCLOSED), capturer);
    EXPECT_EQ(state.getBoard().territoryCount(capturer), 9); // home + 7 perimeter + 1 enclosed
}

TEST(TerritoryResolverTest, captureFlipsOpponentTerritoryInsideTheEnclosedRegion) {
    GameState state = buildStateWithPendingTrail();
    state.getBoard().setTerritoryOwner(ENCLOSED, opponent);
    TerritoryResolver resolver;

    resolver.applyCapture(state, capturer);

    EXPECT_EQ(state.getBoard().territoryOwnerAt(ENCLOSED), capturer);
    // Opponent's own starting cell (7,7), untouched by this capture, is unaffected.
    EXPECT_EQ(state.getBoard().territoryCount(opponent), 1);
}

TEST(TerritoryResolverTest, captureDoesNotClaimOpponentStartingTerritory) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, HOME, {HOME}, ENCLOSED, {ENCLOSED}, 10);
    for (const auto& cell : PERIMETER) {
        state.getBoard().setTrailOwner(cell, capturer);
        state.getPlayer(capturer).getAgent().appendTrail(cell);
    }
    TerritoryResolver resolver;

    resolver.applyCapture(state, capturer);

    EXPECT_EQ(state.getBoard().territoryOwnerAt(ENCLOSED), opponent);
    EXPECT_EQ(state.getBoard().territoryCount(opponent), 1);
}

TEST(TerritoryResolverTest, unrelatedOpponentTrailInsideTheEnclosedRegionIsUntouched) {
    GameState state = buildStateWithPendingTrail();
    state.getBoard().setTrailOwner(ENCLOSED, opponent);
    TerritoryResolver resolver;

    resolver.applyCapture(state, capturer);

    EXPECT_EQ(state.getBoard().territoryOwnerAt(ENCLOSED), capturer);
    EXPECT_EQ(state.getBoard().trailOwnerAt(ENCLOSED), opponent);
}

TEST(TerritoryResolverTest, captureClearsTheCapturersActiveTrail) {
    GameState state = buildStateWithPendingTrail();
    TerritoryResolver resolver;

    resolver.applyCapture(state, capturer);

    EXPECT_TRUE(state.getPlayer(capturer).getAgent().getActiveTrail().empty());
    for (const auto& cell : PERIMETER) {
        EXPECT_FALSE(state.getBoard().trailOwnerAt(cell).has_value());
    }
}

TEST(TerritoryResolverTest, cellsOutsideTheLoopAreNotCaptured) {
    GameState state = buildStateWithPendingTrail();
    GridPosition farAway{0, 0};
    TerritoryResolver resolver;

    resolver.applyCapture(state, capturer);

    EXPECT_FALSE(state.getBoard().territoryOwnerAt(farAway).has_value());
}
