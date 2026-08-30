#include <unordered_set>

#include "testing/TestRunner.hpp"

#include "territorygame/domain/Board.hpp"
#include "territorygame/domain/PlayerId.hpp"

using territorygame::api::GridPosition;
using territorygame::domain::Board;
using territorygame::domain::PlayerId;

namespace {
const PlayerId player0{0};
const PlayerId player1{1};
} // namespace

TEST(BoardTest, newBoardHasNoOwnersAnywhere) {
    Board board(5, 5);
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            GridPosition position{x, y};
            EXPECT_FALSE(board.territoryOwnerAt(position).has_value());
            EXPECT_FALSE(board.trailOwnerAt(position).has_value());
        }
    }
}

TEST(BoardTest, setAndReadTerritoryOwner) {
    Board board(5, 5);
    GridPosition position{2, 2};

    board.setTerritoryOwner(position, player0);

    EXPECT_EQ(board.territoryOwnerAt(position), player0);
}

TEST(BoardTest, cellCanHaveTerritoryOwnerAndDifferentTrailOwnerAtOnce) {
    Board board(5, 5);
    GridPosition position{2, 2};

    board.setTerritoryOwner(position, player0);
    board.setTrailOwner(position, player1);

    EXPECT_EQ(board.territoryOwnerAt(position), player0);
    EXPECT_EQ(board.trailOwnerAt(position), player1);
}

TEST(BoardTest, territoryCountReflectsOwnedCells) {
    Board board(5, 5);
    board.setTerritoryOwner(GridPosition{0, 0}, player0);
    board.setTerritoryOwner(GridPosition{1, 0}, player0);
    board.setTerritoryOwner(GridPosition{2, 0}, player1);

    EXPECT_EQ(board.territoryCount(player0), 2);
    EXPECT_EQ(board.territoryCount(player1), 1);
}

TEST(BoardTest, territoryOfReturnsExactOwnedSet) {
    Board board(5, 5);
    GridPosition a{0, 0};
    GridPosition b{1, 0};
    board.setTerritoryOwner(a, player0);
    board.setTerritoryOwner(b, player0);

    std::unordered_set<GridPosition> expected{a, b};
    EXPECT_EQ(board.territoryOf(player0), expected);
}

TEST(BoardTest, reassigningTerritoryOwnerUpdatesBothOldAndNewCounts) {
    Board board(5, 5);
    GridPosition position{0, 0};
    board.setTerritoryOwner(position, player0);

    board.setTerritoryOwner(position, player1);

    EXPECT_EQ(board.territoryCount(player0), 0);
    EXPECT_EQ(board.territoryCount(player1), 1);
}

TEST(BoardTest, clearAllTerritoryOfRemovesOnlyThatPlayersCells) {
    Board board(5, 5);
    GridPosition a{0, 0};
    GridPosition b{1, 0};
    board.setTerritoryOwner(a, player0);
    board.setTerritoryOwner(b, player1);

    board.clearAllTerritoryOf(player0);

    EXPECT_FALSE(board.territoryOwnerAt(a).has_value());
    EXPECT_EQ(board.territoryOwnerAt(b), player1);
}

TEST(BoardTest, isWithinBoundsRejectsNegativeAndOutOfRangeCoordinates) {
    Board board(5, 5);

    EXPECT_TRUE(board.isWithinBounds(GridPosition{0, 0}));
    EXPECT_TRUE(board.isWithinBounds(GridPosition{4, 4}));
    EXPECT_FALSE(board.isWithinBounds(GridPosition{-1, 0}));
    EXPECT_FALSE(board.isWithinBounds(GridPosition{5, 0}));
    EXPECT_FALSE(board.isWithinBounds(GridPosition{0, 5}));
}
