#include <string>

#include "testing/TestRunner.hpp"

#include "territorygame/gui/SnapshotHistory.hpp"

using territorygame::gui::SnapshotHistory;

namespace {
SnapshotHistory<std::string> historyWith(int count) {
    SnapshotHistory<std::string> history;
    for (int i = 1; i <= count; i++) {
        history.record("s" + std::to_string(i));
    }
    return history;
}
} // namespace

TEST(SnapshotHistoryTest, recordedSnapshotBecomesCurrent) {
    SnapshotHistory<std::string> history;
    history.record("initial");
    EXPECT_EQ(history.current(), "initial");
}

TEST(SnapshotHistoryTest, backShowsThePreviousSnapshot) {
    SnapshotHistory<std::string> history;
    history.record("first");
    history.record("second");
    history.back();
    EXPECT_EQ(history.current(), "first");
}

TEST(SnapshotHistoryTest, backAtTheStartStaysOnTheFirstSnapshot) {
    SnapshotHistory<std::string> history;
    history.record("only");
    history.back();
    EXPECT_EQ(history.current(), "only");
}

TEST(SnapshotHistoryTest, forwardAfterBackReturnsTowardLive) {
    SnapshotHistory<std::string> history;
    history.record("first");
    history.record("second");
    history.back();
    history.forward();
    EXPECT_EQ(history.current(), "second");
}

TEST(SnapshotHistoryTest, recordingWhileReviewingJumpsToTheNewLiveSnapshot) {
    SnapshotHistory<std::string> history;
    history.record("first");
    history.record("second");
    history.record("third");
    history.back();
    history.back();
    history.record("fourth");
    EXPECT_EQ(history.current(), "fourth");
}

TEST(SnapshotHistoryTest, clearDiscardsHistorySoTheNextRecordIsCurrent) {
    SnapshotHistory<std::string> history;
    history.record("old");
    history.clear();
    history.record("fresh");
    EXPECT_EQ(history.current(), "fresh");
}

TEST(SnapshotHistoryTest, canGoBackAndForwardTrackTheCursor) {
    SnapshotHistory<std::string> history;
    history.record("first");
    history.record("second");
    EXPECT_FALSE(history.canGoForward());
    EXPECT_TRUE(history.canGoBack());
    history.back();
    EXPECT_TRUE(history.canGoForward());
    EXPECT_FALSE(history.canGoBack());
}

TEST(SnapshotHistoryTest, isAtLiveUntilTheUserStepsBack) {
    SnapshotHistory<std::string> history;
    history.record("first");
    history.record("second");
    EXPECT_TRUE(history.isAtLive());
    history.back();
    EXPECT_FALSE(history.isAtLive());
    history.forward();
    EXPECT_TRUE(history.isAtLive());
}

TEST(SnapshotHistoryTest, positionIsOneBasedForStatusText) {
    SnapshotHistory<std::string> history;
    history.record("first");
    history.record("second");
    history.record("third");
    EXPECT_EQ(history.position(), 3);
    EXPECT_EQ(history.size(), 3);
    history.back();
    EXPECT_EQ(history.position(), 2);
    EXPECT_EQ(history.size(), 3);
}

TEST(SnapshotHistoryTest, backByTenMovesTenSnapshots) {
    SnapshotHistory<std::string> history = historyWith(15);
    history.back(10);
    EXPECT_EQ(history.current(), "s5");
}

TEST(SnapshotHistoryTest, backByTenClampsAtTheStart) {
    SnapshotHistory<std::string> history = historyWith(4);
    history.back(10);
    EXPECT_EQ(history.current(), "s1");
}

TEST(SnapshotHistoryTest, forwardByTenMovesTenSnapshots) {
    SnapshotHistory<std::string> history = historyWith(15);
    history.back(14);
    history.forward(10);
    EXPECT_EQ(history.current(), "s11");
}

TEST(SnapshotHistoryTest, forwardByTenClampsAtLive) {
    SnapshotHistory<std::string> history = historyWith(4);
    history.back(2);
    history.forward(10);
    EXPECT_EQ(history.current(), "s4");
}
