#include <vector>

#include "testing/TestRunner.hpp"

#include "territorygame/engine/GameSnapshot.hpp"

using territorygame::api::GridPosition;
using territorygame::api::MoveResult;
using territorygame::domain::PlayerId;
using territorygame::engine::GameSnapshot;

namespace {
const PlayerId player0{0};

std::vector<std::vector<GameSnapshot::CellSnapshot>> cellsWithOneOwnedCell() {
    std::vector<std::vector<GameSnapshot::CellSnapshot>> cells(
        2, std::vector<GameSnapshot::CellSnapshot>(2, GameSnapshot::CellSnapshot{std::nullopt, std::nullopt}));
    cells[0][0] = GameSnapshot::CellSnapshot{player0, std::nullopt};
    return cells;
}

GameSnapshot buildSnapshot(std::vector<std::vector<GameSnapshot::CellSnapshot>> cells) {
    return GameSnapshot{
        2, 2, std::move(cells), {}, player0, MoveResult::MOVED, 3, false, std::nullopt};
}

} // namespace

TEST(GameSnapshotTest, mutatingTheOriginalArrayAfterConstructionDoesNotAffectTheSnapshot) {
    auto cells = cellsWithOneOwnedCell();
    GameSnapshot snapshot = buildSnapshot(cells); // copied by value into the snapshot

    cells[0][0] = GameSnapshot::CellSnapshot{std::nullopt, std::nullopt};

    EXPECT_EQ(snapshot.cells[0][0].territoryOwner, player0);
}

TEST(GameSnapshotTest, equalSnapshotsWithDifferentCellArrayInstancesAreEqual) {
    GameSnapshot a = buildSnapshot(cellsWithOneOwnedCell());
    GameSnapshot b = buildSnapshot(cellsWithOneOwnedCell());

    EXPECT_TRUE(a == b);
}

TEST(GameSnapshotTest, snapshotsWithDifferentCellsAreNotEqual) {
    std::vector<std::vector<GameSnapshot::CellSnapshot>> empty(
        2, std::vector<GameSnapshot::CellSnapshot>(2, GameSnapshot::CellSnapshot{std::nullopt, std::nullopt}));
    GameSnapshot a = buildSnapshot(cellsWithOneOwnedCell());
    GameSnapshot b = buildSnapshot(empty);

    EXPECT_TRUE(a != b);
}

TEST(GameSnapshotTest, playerSnapshotScoreCombinesTerritoryAndKillsWithABonus) {
    GameSnapshot::PlayerSnapshot player{player0, GridPosition{0, 0}, 5, 2, 0, 10, {}, std::nullopt};

    EXPECT_EQ(player.score(), 25); // 5 territory + 2 kills * 10
}
