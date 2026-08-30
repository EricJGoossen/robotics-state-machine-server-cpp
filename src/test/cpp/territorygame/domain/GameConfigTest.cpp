// Covers the value validation GameConfig::create() performs (mirroring
// Java's record compact constructor, which runs on every construction
// path); a direct call is enough to exercise it without needing a
// properties file.

#include <vector>

#include "testing/TestRunner.hpp"

#include "territorygame/domain/GameConfig.hpp"

using territorygame::api::GridPosition;
using territorygame::domain::GameConfig;

namespace {

GameConfig validConfigWith(
    int boardWidth, int boardHeight, int visibilityWindowSize, int turnsPerPlayer,
    std::vector<GridPosition> respawnPositions, int startingTerritorySize, int autoPlayTurnDelayMillis,
    int maxAttemptsPerTurn) {
    return GameConfig::create(
        boardWidth, boardHeight, visibilityWindowSize, turnsPerPlayer, std::move(respawnPositions),
        startingTerritorySize, autoPlayTurnDelayMillis, maxAttemptsPerTurn, std::vector<int64_t>{1, 2});
}

} // namespace

TEST(GameConfigTest, rejectsNonPositiveBoardDimensions) {
    EXPECT_THROWS(validConfigWith(
        0, 10, 5, 10, std::vector<GridPosition>{GridPosition{1, 1}, GridPosition{8, 8}}, 1, 0, 20));
}

TEST(GameConfigTest, rejectsEvenVisibilityWindowSize) {
    EXPECT_THROWS(validConfigWith(
        10, 10, 4, 10, std::vector<GridPosition>{GridPosition{1, 1}, GridPosition{8, 8}}, 1, 0, 20));
}

TEST(GameConfigTest, rejectsVisibilityWindowLargerThanTheBoard) {
    EXPECT_THROWS(validConfigWith(
        10, 10, 11, 10, std::vector<GridPosition>{GridPosition{1, 1}, GridPosition{8, 8}}, 1, 0, 20));
}

TEST(GameConfigTest, rejectsNonPositiveTurnsPerPlayer) {
    EXPECT_THROWS(validConfigWith(
        10, 10, 5, 0, std::vector<GridPosition>{GridPosition{1, 1}, GridPosition{8, 8}}, 1, 0, 20));
}

TEST(GameConfigTest, rejectsNonPositiveStartingTerritorySize) {
    EXPECT_THROWS(validConfigWith(
        10, 10, 5, 10, std::vector<GridPosition>{GridPosition{1, 1}, GridPosition{8, 8}}, 0, 0, 20));
}

TEST(GameConfigTest, rejectsNegativeAutoPlayDelay) {
    EXPECT_THROWS(validConfigWith(
        10, 10, 5, 10, std::vector<GridPosition>{GridPosition{1, 1}, GridPosition{8, 8}}, 1, -1, 20));
}

TEST(GameConfigTest, rejectsNonPositiveMaxAttemptsPerTurn) {
    EXPECT_THROWS(validConfigWith(
        10, 10, 5, 10, std::vector<GridPosition>{GridPosition{1, 1}, GridPosition{8, 8}}, 1, 0, 0));
}

TEST(GameConfigTest, rejectsOutOfBoundsRespawnPosition) {
    EXPECT_THROWS(validConfigWith(
        10, 10, 5, 10, std::vector<GridPosition>{GridPosition{1, 1}, GridPosition{20, 20}}, 1, 0, 20));
}

TEST(GameConfigTest, rejectsOverlappingStartingTerritories) {
    EXPECT_THROWS(validConfigWith(
        10, 10, 5, 10, std::vector<GridPosition>{GridPosition{4, 4}, GridPosition{5, 5}}, 3, 0, 20));
}

TEST(GameConfigTest, rejectsControllerSeedCountMismatch) {
    EXPECT_THROWS(GameConfig::create(
        10, 10, 5, 10, std::vector<GridPosition>{GridPosition{1, 1}, GridPosition{8, 8}}, 1, 0, 20,
        std::vector<int64_t>{1}));
}

TEST(GameConfigTest, acceptsAWellFormedConfig) {
    EXPECT_NO_THROW(validConfigWith(
        10, 10, 5, 10, std::vector<GridPosition>{GridPosition{1, 1}, GridPosition{8, 8}}, 1, 0, 20));
}
