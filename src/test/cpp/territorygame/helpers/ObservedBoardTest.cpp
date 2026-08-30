#include "testing/TestRunner.hpp"

#include "territorygame/helpers/ObservedBoard.hpp"

using territorygame::api::CellViewType;
using territorygame::api::GridPosition;
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
    std::vector<std::vector<VisibleCell>> grid{{VisibleCell{position, CellViewType::SELF_TERRITORY}}};

    board.update(grid);

    EXPECT_TRUE(board.hasObserved(position));
    EXPECT_EQ(board.get(position), std::optional<CellViewType>(CellViewType::SELF_TERRITORY));
}

TEST(ObservedBoardTest, laterUpdateOverwritesEarlierValueForSameCell) {
    ObservedBoard board(5, 5);
    GridPosition position{1, 1};

    board.update({{VisibleCell{position, CellViewType::FREE}}});
    board.update({{VisibleCell{position, CellViewType::OPPONENT_TERRITORY}}});

    EXPECT_EQ(board.get(position), std::optional<CellViewType>(CellViewType::OPPONENT_TERRITORY));
}

TEST(ObservedBoardTest, updateDoesNotAffectCellsOutsideTheGivenGrid) {
    ObservedBoard board(5, 5);
    GridPosition observed{1, 1};
    GridPosition untouched{3, 3};
    board.update({{VisibleCell{observed, CellViewType::SELF_TERRITORY}}});

    EXPECT_FALSE(board.hasObserved(untouched));
}

TEST(ObservedBoardTest, clearForgetsAllPreviouslyObservedCells) {
    ObservedBoard board(5, 5);
    GridPosition position{1, 1};
    board.update({{VisibleCell{position, CellViewType::SELF_TERRITORY}}});

    board.clear();

    EXPECT_FALSE(board.hasObserved(position));
}

TEST(ObservedBoardTest, getReturnsEmptyForOutOfBoundsPosition) {
    ObservedBoard board(5, 5);

    EXPECT_FALSE(board.get(GridPosition{-1, 0}).has_value());
    EXPECT_FALSE(board.hasObserved(GridPosition{10, 10}));
}
