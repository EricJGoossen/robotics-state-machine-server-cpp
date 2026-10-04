#include <optional>
#include <vector>

#include "testing/TestRunner.hpp"

#include "territorygame/api/MoveResult.hpp"
#include "territorygame/domain/PlayerId.hpp"
#include "territorygame/engine/GameSnapshot.hpp"
#include "territorygame/gui/SnapshotHistory.hpp"

using territorygame::api::MoveResult;
using territorygame::domain::PlayerId;
using territorygame::engine::GameSnapshot;
using territorygame::gui::SnapshotHistory;

namespace {
GameSnapshot snapshot(std::optional<MoveResult> lastMove) {
    PlayerId player0{0};
    GameSnapshot snap;
    snap.width = 1;
    snap.height = 1;
    snap.cells = {{GameSnapshot::CellSnapshot{std::nullopt, std::nullopt}}};
    snap.players = {};
    snap.activePlayerId = player0;
    snap.lastMoveResult = lastMove;
    snap.visibilityWindowSize = 3;
    snap.gameOver = false;
    snap.errorMessage = std::nullopt;
    return snap;
}
} // namespace

TEST(EngineSnapshotIngestTest, snapshotWithNoLastMoveReplacesHistory) {
    SnapshotHistory<GameSnapshot> history;
    GameSnapshot afterTurn = snapshot(MoveResult::MOVED);
    GameSnapshot afterReset = snapshot(std::nullopt);

    territorygame::gui::ingestEngineSnapshot(history, afterTurn);
    territorygame::gui::ingestEngineSnapshot(history, afterReset);

    EXPECT_EQ(history.current(), afterReset);
    EXPECT_FALSE(history.canGoBack());
}

TEST(EngineSnapshotIngestTest, snapshotAfterAMoveAppendsToHistory) {
    SnapshotHistory<GameSnapshot> history;
    GameSnapshot first = snapshot(std::nullopt);
    GameSnapshot second = snapshot(MoveResult::MOVED);

    territorygame::gui::ingestEngineSnapshot(history, first);
    territorygame::gui::ingestEngineSnapshot(history, second);

    EXPECT_EQ(history.current(), second);
    EXPECT_TRUE(history.canGoBack());
}
