#include <vector>

#include "testing/TestRunner.hpp"

#include "territorygame/TestGames.hpp"
#include "territorygame/visibility/VisibilityService.hpp"

using namespace territorygame::api;
using territorygame::domain::GameState;
using territorygame::domain::PlayerId;
using territorygame::visibility::VisibilityService;

namespace {
const PlayerId player0{0};
const PlayerId player1{1};

VisibleCell cellAt(const std::vector<std::vector<VisibleCell>>& grid, GridPosition position) {
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            if (cell.position == position) {
                return cell;
            }
        }
    }
    throw std::runtime_error("Position not in visible grid");
}

} // namespace

TEST(VisibilityServiceTest, windowIsFullSizeWhenFarFromEveryEdge) {
    GameState state = territorygame::test::twoPlayerState(
        20, 20, GridPosition{10, 10}, {GridPosition{10, 10}}, GridPosition{19, 19}, {GridPosition{19, 19}}, 10);
    VisibilityService service(5); // half=2

    auto grid = service.computeVisibleGrid(state, player0);

    EXPECT_EQ(grid.size(), static_cast<size_t>(5));
    EXPECT_EQ(grid[0].size(), static_cast<size_t>(5));
    EXPECT_EQ(grid[2][2].position, (GridPosition{10, 10}));
}

TEST(VisibilityServiceTest, windowClipsAtTheBoardCorner) {
    GameState state = territorygame::test::twoPlayerState(
        20, 20, GridPosition{0, 0}, {GridPosition{0, 0}}, GridPosition{19, 19}, {GridPosition{19, 19}}, 10);
    VisibilityService service(5); // half=2

    auto grid = service.computeVisibleGrid(state, player0);

    // Window would be x:[-2,2], y:[-2,2]; clipped to x:[0,2], y:[0,2] -> 3x3.
    EXPECT_EQ(grid.size(), static_cast<size_t>(3));
    EXPECT_EQ(grid[0].size(), static_cast<size_t>(3));
    EXPECT_EQ(grid[0][0].position, (GridPosition{0, 0}));
}

TEST(VisibilityServiceTest, ownershipTranslatesToSelfAndOpponentRelativeToViewer) {
    GridPosition player0Territory{4, 5};
    GridPosition player1Territory{7, 6};
    GameState state = territorygame::test::twoPlayerState(
        10, 10, GridPosition{5, 5}, {GridPosition{5, 5}, player0Territory},
        GridPosition{6, 6}, {GridPosition{6, 6}, player1Territory}, 10);
    VisibilityService service(9);

    auto fromPlayer0 = service.computeVisibleGrid(state, player0);
    auto fromPlayer1 = service.computeVisibleGrid(state, player1);

    EXPECT_EQ(cellAt(fromPlayer0, player0Territory).occupant, OccupantView::EMPTY);
    EXPECT_EQ(cellAt(fromPlayer0, player0Territory).territory, TerritoryView::SELF);
    EXPECT_EQ(cellAt(fromPlayer0, player1Territory).occupant, OccupantView::EMPTY);
    EXPECT_EQ(cellAt(fromPlayer0, player1Territory).territory, TerritoryView::OPPONENT);
    // Same cells, viewed by the other player, flip labels.
    EXPECT_EQ(cellAt(fromPlayer1, player0Territory).occupant, OccupantView::EMPTY);
    EXPECT_EQ(cellAt(fromPlayer1, player0Territory).territory, TerritoryView::OPPONENT);
    EXPECT_EQ(cellAt(fromPlayer1, player1Territory).occupant, OccupantView::EMPTY);
    EXPECT_EQ(cellAt(fromPlayer1, player1Territory).territory, TerritoryView::SELF);
}

TEST(VisibilityServiceTest, agentPositionsAreReportedAsSelfOrOpponentAgent) {
    GameState state = territorygame::test::twoPlayerState(
        10, 10, GridPosition{5, 5}, {GridPosition{5, 5}}, GridPosition{6, 5}, {GridPosition{6, 5}}, 10);
    VisibilityService service(9);

    auto grid = service.computeVisibleGrid(state, player0);

    EXPECT_EQ(cellAt(grid, GridPosition{5, 5}).occupant, OccupantView::SELF_AGENT);
    EXPECT_EQ(cellAt(grid, GridPosition{6, 5}).occupant, OccupantView::OPPONENT_AGENT);
}

TEST(VisibilityServiceTest, trailOccupantDoesNotHideTerritory) {
    GameState state = territorygame::test::twoPlayerState(
        10, 10, GridPosition{5, 5}, {GridPosition{5, 5}}, GridPosition{0, 0}, {}, 10);
    GridPosition trailOverTerritory{4, 5};
    state.getBoard().setTerritoryOwner(trailOverTerritory, player0);
    state.getBoard().setTrailOwner(trailOverTerritory, player0);

    VisibilityService service(9);
    VisibleCell cell = cellAt(service.computeVisibleGrid(state, player0), trailOverTerritory);

    EXPECT_EQ(cell.occupant, OccupantView::SELF_TRAIL);
    EXPECT_EQ(cell.territory, TerritoryView::SELF);
}

TEST(VisibilityServiceTest, opponentTrailOnViewerLandReportsBothLayers) {
    GameState state = territorygame::test::twoPlayerState(
        10, 10, GridPosition{5, 5}, {GridPosition{5, 5}}, GridPosition{0, 0}, {}, 10);
    GridPosition cut{4, 5};
    state.getBoard().setTerritoryOwner(cut, player0);
    state.getBoard().setTrailOwner(cut, player1);

    VisibilityService service(9);
    VisibleCell cell = cellAt(service.computeVisibleGrid(state, player0), cut);

    EXPECT_EQ(cell.occupant, OccupantView::OPPONENT_TRAIL);
    EXPECT_EQ(cell.territory, TerritoryView::SELF);
}

TEST(VisibilityServiceTest, unoccupiedUnownedCellIsEmptyAndUnowned) {
    GameState state = territorygame::test::twoPlayerState(
        10, 10, GridPosition{5, 5}, {GridPosition{5, 5}}, GridPosition{0, 0}, {GridPosition{0, 0}}, 10);
    VisibilityService service(9);

    VisibleCell cell = cellAt(service.computeVisibleGrid(state, player0), GridPosition{6, 5});

    EXPECT_EQ(cell.occupant, OccupantView::EMPTY);
    EXPECT_EQ(cell.territory, TerritoryView::UNOWNED);
}
