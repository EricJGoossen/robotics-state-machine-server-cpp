// The Java original's GameEngine runs commands on a background thread, so
// its tests observe results by polling a blocking queue with a timeout.
// This C++ port's GameEngine now does the same (see GameEngine.hpp), so
// SnapshotCollector below is a small thread-safe sink that lets a test
// block until the expected number of snapshots (or a game-over snapshot)
// has actually been published, instead of racing the worker thread.

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "testing/TestRunner.hpp"

#include "territorygame/api/AgentController.hpp"
#include "territorygame/domain/GameConfig.hpp"
#include "territorygame/engine/GameEngine.hpp"
#include "territorygame/engine/GameObserver.hpp"
#include "territorygame/engine/GameSnapshot.hpp"

using namespace territorygame::api;
using territorygame::domain::GameConfig;
using territorygame::engine::GameEngine;
using territorygame::engine::GameObserver;
using territorygame::engine::GameSnapshot;

namespace {

constexpr auto TEST_TIMEOUT = std::chrono::seconds(5);

class AlwaysMoveController final : public AgentController {
public:
    explicit AlwaysMoveController(Direction direction) : direction_(direction) {}
    void takeTurn(GameApi& game) override { game.move(direction_); }

private:
    Direction direction_;
};

class SnapshotCollector final : public GameObserver {
public:
    void onGameStateChanged(const GameSnapshot& snapshot) override {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshots_.push_back(snapshot);
        cv_.notify_all();
    }

    GameSnapshot front() {
        std::lock_guard<std::mutex> lock(mutex_);
        return snapshots_.front();
    }

    GameSnapshot back() {
        std::lock_guard<std::mutex> lock(mutex_);
        return snapshots_.back();
    }

    // Blocks until at least `count` snapshots have been published.
    void waitForAtLeast(size_t count) {
        std::unique_lock<std::mutex> lock(mutex_);
        bool reached = cv_.wait_for(lock, TEST_TIMEOUT, [&] { return snapshots_.size() >= count; });
        if (!reached) {
            TG_FAIL("timed out waiting for at least " << count << " snapshots; got " << snapshots_.size());
        }
    }

    // Blocks until a game-over snapshot has been published.
    void waitForGameOver() {
        std::unique_lock<std::mutex> lock(mutex_);
        bool reached = cv_.wait_for(lock, TEST_TIMEOUT, [&] { return !snapshots_.empty() && snapshots_.back().gameOver; });
        if (!reached) {
            TG_FAIL("timed out waiting for the match to reach game over");
        }
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<GameSnapshot> snapshots_;
};

} // namespace

TEST(GameEngineTest, matchRunsToCompletionWithAlternatingTurnsAndCorrectFinalCounts) {
    GameConfig config = GameConfig::create(
        8, 8, 5, 2, {GridPosition{1, 1}, GridPosition{6, 6}}, 1, 0, // no auto-play delay in tests
        20, {1, 2});
    std::vector<std::shared_ptr<AgentController>> controllers{
        std::make_shared<AlwaysMoveController>(Direction::EAST),
        std::make_shared<AlwaysMoveController>(Direction::WEST)};
    GameEngine engine(config, controllers);
    SnapshotCollector collector;
    engine.addObserver(&collector);
    engine.reset(controllers);
    collector.waitForAtLeast(1);

    EXPECT_FALSE(collector.front().gameOver);

    engine.start();
    collector.waitForGameOver();

    GameSnapshot last = collector.back();
    EXPECT_TRUE(last.gameOver);
    for (const auto& player : last.players) {
        EXPECT_EQ(player.remainingTurns, 0);
        EXPECT_EQ(player.territoryCount, 1); // neither move returned home to capture
    }
}

TEST(GameEngineTest, stepRunsExactlyOneTurn) {
    GameConfig config = GameConfig::create(
        8, 8, 5, 10, {GridPosition{1, 1}, GridPosition{6, 6}}, 1, 0, // no auto-play delay in tests
        20, {1, 2});
    std::vector<std::shared_ptr<AgentController>> controllers{
        std::make_shared<AlwaysMoveController>(Direction::EAST),
        std::make_shared<AlwaysMoveController>(Direction::WEST)};
    GameEngine engine(config, controllers);
    SnapshotCollector collector;
    engine.addObserver(&collector);
    engine.reset(controllers); // publishes the initial snapshot
    collector.waitForAtLeast(1);

    engine.step();
    collector.waitForAtLeast(2);
    GameSnapshot afterOneStep = collector.back();

    // Only player0 (the first active player) should have moved.
    EXPECT_EQ(afterOneStep.players[0].remainingTurns, 9);
    EXPECT_EQ(afterOneStep.players[1].remainingTurns, 10);
    EXPECT_EQ(afterOneStep.activePlayerId, afterOneStep.players[1].id);
}

TEST(GameEngineTest, resetWithNewControllersReplacesThePreviousOnes) {
    GameConfig config = GameConfig::create(
        8, 8, 5, 10, {GridPosition{1, 1}, GridPosition{6, 6}}, 1, 0, // no auto-play delay in tests
        20, {1, 2});
    std::vector<std::shared_ptr<AgentController>> initialControllers{
        std::make_shared<AlwaysMoveController>(Direction::EAST),
        std::make_shared<AlwaysMoveController>(Direction::WEST)};
    GameEngine engine(config, initialControllers);
    SnapshotCollector collector;
    engine.addObserver(&collector);
    engine.reset(initialControllers);
    collector.waitForAtLeast(1);

    std::vector<std::shared_ptr<AgentController>> newControllers{
        std::make_shared<AlwaysMoveController>(Direction::SOUTH),
        std::make_shared<AlwaysMoveController>(Direction::NORTH)};
    engine.reset(newControllers);
    collector.waitForAtLeast(2);
    GameSnapshot afterReset = collector.back();
    EXPECT_FALSE(afterReset.gameOver);

    engine.step();
    collector.waitForAtLeast(3);
    GameSnapshot afterStep = collector.back();

    // Player0 started at (1,1); with the new controllers it should have moved
    // SOUTH to (1,2), not EAST to (2,1).
    EXPECT_EQ(afterStep.players[0].position, (GridPosition{1, 2}));
}
