#include <algorithm>
#include <random>
#include <unordered_set>
#include <vector>

#include "testing/TestRunner.hpp"

#include "territorygame/api/GameApi.hpp"
#include "territorygame/helpers/MovementUtils.hpp"

using namespace territorygame::api;
using territorygame::helpers::MovementUtils;

namespace {

// Minimal GameApi test double exposing only what MovementUtils reads.
class StubGameApi final : public GameApi {
public:
    StubGameApi(GridPosition position, int width, int height, std::vector<std::vector<VisibleCell>> visibleGrid)
        : position_(position), width_(width), height_(height), visibleGrid_(std::move(visibleGrid)) {}

    GridPosition getAgentPosition() const override { return position_; }
    GridPosition getRespawnPosition() const override { return position_; }
    int getOwnedTerritoryCellCount() const override { return 0; }
    int getOpponentTerritoryCellCount() const override { return 0; }
    int getRemainingTurns() const override { return 0; }
    std::vector<GridPosition> getActiveTrail() const override { return {}; }
    std::vector<std::vector<VisibleCell>> getVisibleGrid() const override { return visibleGrid_; }
    int getBoardWidth() const override { return width_; }
    int getBoardHeight() const override { return height_; }
    MoveResult move(Direction) override { return MoveResult::INVALID; }

private:
    GridPosition position_;
    int width_;
    int height_;
    std::vector<std::vector<VisibleCell>> visibleGrid_;
};

} // namespace

TEST(MovementUtilsTest, nextPositionMovesOneCellPerDirection) {
    GridPosition start{5, 5};

    EXPECT_EQ(MovementUtils::nextPosition(start, Direction::NORTH), (GridPosition{5, 4}));
    EXPECT_EQ(MovementUtils::nextPosition(start, Direction::SOUTH), (GridPosition{5, 6}));
    EXPECT_EQ(MovementUtils::nextPosition(start, Direction::EAST), (GridPosition{6, 5}));
    EXPECT_EQ(MovementUtils::nextPosition(start, Direction::WEST), (GridPosition{4, 5}));
}

TEST(MovementUtilsTest, isWithinBoardChecksBothAxes) {
    EXPECT_TRUE(MovementUtils::isWithinBoard(GridPosition{0, 0}, 5, 5));
    EXPECT_TRUE(MovementUtils::isWithinBoard(GridPosition{4, 4}, 5, 5));
    EXPECT_FALSE(MovementUtils::isWithinBoard(GridPosition{-1, 0}, 5, 5));
    EXPECT_FALSE(MovementUtils::isWithinBoard(GridPosition{5, 0}, 5, 5));
    EXPECT_FALSE(MovementUtils::isWithinBoard(GridPosition{0, 5}, 5, 5));
}

TEST(MovementUtilsTest, isValidBoardMoveChecksDestinationBounds) {
    EXPECT_TRUE(MovementUtils::isValidBoardMove(GridPosition{0, 0}, Direction::EAST, 5, 5));
    EXPECT_FALSE(MovementUtils::isValidBoardMove(GridPosition{0, 0}, Direction::NORTH, 5, 5));
    EXPECT_FALSE(MovementUtils::isValidBoardMove(GridPosition{0, 0}, Direction::WEST, 5, 5));
}

TEST(MovementUtilsTest, manhattanDistanceIsGridDistance) {
    EXPECT_EQ(MovementUtils::manhattanDistance(GridPosition{0, 0}, GridPosition{3, 4}), 7);
    EXPECT_EQ(MovementUtils::manhattanDistance(GridPosition{2, 2}, GridPosition{2, 2}), 0);
}

TEST(MovementUtilsTest, isValidMoveRejectsOutOfBoundsWithoutNeedingVisibleGrid) {
    StubGameApi game(GridPosition{0, 0}, 5, 5, {});

    EXPECT_FALSE(MovementUtils::isValidMove(game, Direction::NORTH));
}

TEST(MovementUtilsTest, isValidMoveRejectsOpponentAgentCell) {
    GridPosition position{2, 2};
    GridPosition opponentAt{3, 2};
    std::vector<std::vector<VisibleCell>> grid{{VisibleCell{opponentAt, CellViewType::OPPONENT_AGENT}}};
    StubGameApi game(position, 5, 5, grid);

    EXPECT_FALSE(MovementUtils::isValidMove(game, Direction::EAST));
}

TEST(MovementUtilsTest, isValidMoveAcceptsFreeInBoundsCell) {
    GridPosition position{2, 2};
    GridPosition destination{3, 2};
    std::vector<std::vector<VisibleCell>> grid{{VisibleCell{destination, CellViewType::FREE}}};
    StubGameApi game(position, 5, 5, grid);

    EXPECT_TRUE(MovementUtils::isValidMove(game, Direction::EAST));
}

TEST(MovementUtilsTest, findCellReturnsTheCellAtAMatchingPosition) {
    GridPosition target{3, 2};
    std::vector<std::vector<VisibleCell>> grid{{VisibleCell{target, CellViewType::OPPONENT_TERRITORY}}};

    auto found = MovementUtils::findCell(grid, target);

    EXPECT_TRUE(found.has_value());
    EXPECT_EQ(found->type, CellViewType::OPPONENT_TERRITORY);
}

TEST(MovementUtilsTest, findCellReturnsEmptyWhenPositionIsNotInTheGrid) {
    std::vector<std::vector<VisibleCell>> grid{{VisibleCell{GridPosition{3, 2}, CellViewType::FREE}}};

    EXPECT_FALSE(MovementUtils::findCell(grid, GridPosition{9, 9}).has_value());
}

TEST(MovementUtilsTest, validDirectionsExcludesOutOfBoundsAndOpponentAgentCells) {
    GridPosition position{0, 0};
    GridPosition east{1, 0};
    GridPosition south{0, 1};
    std::vector<std::vector<VisibleCell>> grid{
        {VisibleCell{position, CellViewType::SELF_AGENT}, VisibleCell{east, CellViewType::OPPONENT_AGENT}},
        {VisibleCell{south, CellViewType::FREE}, VisibleCell{GridPosition{1, 1}, CellViewType::FREE}}};
    StubGameApi game(position, 5, 5, grid);

    auto valid = MovementUtils::validDirections(game);

    std::unordered_set<int> actual;
    for (Direction d : valid) actual.insert(static_cast<int>(d));
    std::unordered_set<int> expected{static_cast<int>(Direction::SOUTH)};
    EXPECT_EQ(actual, expected);
}

TEST(MovementUtilsTest, randomDirectionReturnsOneOfTheFourDirections) {
    std::mt19937_64 random(1);
    Direction direction = MovementUtils::randomDirection(random);

    EXPECT_TRUE(std::find(DIRECTIONS.begin(), DIRECTIONS.end(), direction) != DIRECTIONS.end());
}
