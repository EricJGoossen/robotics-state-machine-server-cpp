#include "testing/TestRunner.hpp"

#include "territorygame/helpers/ObservedBoard.hpp"

using territorygame::api::GridPosition;
using territorygame::api::OccupantView;
using territorygame::api::TerritoryView;
using territorygame::api::VisibleCell;
using territorygame::helpers::ObservedBoard;

TEST(ObservedBoardTest, unobservedCellHasNoValue) {
    ObservedBoard board(5, 5);
    GridPosition position{1, 1};

    EXPECT_FALSE(board.hasObserved(position));
    EXPECT_FALSE(board.get(position).has_value());
}

TEST(ObservedBoardTest, updateStoresLatestValuePerCell) {
    ObservedBoard board(5, 5);
    GridPosition position{1, 1};
    VisibleCell stored{position, OccupantView::EMPTY, TerritoryView::SELF};
    std::vector<std::vector<VisibleCell>> grid{{stored}};

    board.update(grid);

    EXPECT_TRUE(board.hasObserved(position));
    auto found = board.get(position);
    EXPECT_TRUE(found.has_value());
    EXPECT_EQ(found->occupant, stored.occupant);
    EXPECT_EQ(found->territory, stored.territory);
}

TEST(ObservedBoardTest, laterUpdateOverwritesEarlierValueForSameCell) {
    ObservedBoard board(5, 5);
    GridPosition position{1, 1};
    VisibleCell later{position, OccupantView::EMPTY, TerritoryView::OPPONENT};

    board.update({{VisibleCell{position, OccupantView::EMPTY, TerritoryView::UNOWNED}}});
    board.update({{later}});

    auto found = board.get(position);
    EXPECT_TRUE(found.has_value());
    EXPECT_EQ(found->occupant, later.occupant);
    EXPECT_EQ(found->territory, later.territory);
}

TEST(ObservedBoardTest, updateDoesNotAffectCellsOutsideTheGivenGrid) {
    ObservedBoard board(5, 5);
    GridPosition observed{1, 1};
    GridPosition untouched{3, 3};
    board.update({{VisibleCell{observed, OccupantView::EMPTY, TerritoryView::SELF}}});

    EXPECT_FALSE(board.hasObserved(untouched));
}

TEST(ObservedBoardTest, clearForgetsAllPreviouslyObservedCells) {
    ObservedBoard board(5, 5);
    GridPosition position{1, 1};
    board.update({{VisibleCell{position, OccupantView::EMPTY, TerritoryView::SELF}}});

    board.clear();

    EXPECT_FALSE(board.hasObserved(position));
}

TEST(ObservedBoardTest, getReturnsEmptyForOutOfBoundsPosition) {
    ObservedBoard board(5, 5);

    EXPECT_FALSE(board.get(GridPosition{-1, 0}).has_value());
    EXPECT_FALSE(board.hasObserved(GridPosition{10, 10}));
}
