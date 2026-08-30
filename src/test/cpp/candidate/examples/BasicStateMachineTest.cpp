// Confirms the example agent can play many turns against itself without
// any framework error. GameEngine runs commands on its own background
// thread (see GameEngineTest.cpp), so this polls for the expected snapshots
// instead of assuming reset()/start() have already taken effect when they
// return.

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include "testing/TestRunner.hpp"

#include "candidate/examples/BasicStateMachine.hpp"
#include "territorygame/api/AgentController.hpp"
#include "territorygame/domain/GameConfig.hpp"
#include "territorygame/engine/GameEngine.hpp"
#include "territorygame/engine/GameObserver.hpp"
#include "territorygame/engine/GameSnapshot.hpp"

using namespace territorygame::api;
using candidate::examples::BasicStateMachine;
using territorygame::domain::GameConfig;
using territorygame::engine::GameEngine;
using territorygame::engine::GameObserver;
using territorygame::engine::GameSnapshot;

namespace {

constexpr auto TEST_TIMEOUT = std::chrono::seconds(5);

class SnapshotCollector final : public GameObserver {
public:
    void onGameStateChanged(const GameSnapshot& snapshot) override {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshots_.push_back(snapshot);
        cv_.notify_all();
    }

    // Blocks until at least one snapshot has been published.
    void waitForFirstSnapshot() {
        std::unique_lock<std::mutex> lock(mutex_);
        bool reached = cv_.wait_for(lock, TEST_TIMEOUT, [&] { return !snapshots_.empty(); });
        if (!reached) {
            TG_FAIL("timed out waiting for the initial snapshot");
        }
    }

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

TEST(BasicStateMachineTest, playsManyTurnsAgainstItselfWithoutFrameworkErrors) {
    GameConfig config = GameConfig::create(
        20, 20, 11, 40, {GridPosition{4, 10}, GridPosition{15, 10}}, 3, 0, // no auto-play delay in tests
        20, {1, 2});
    std::vector<std::shared_ptr<AgentController>> controllers{
        std::make_shared<BasicStateMachine>(), std::make_shared<BasicStateMachine>()};
    GameEngine engine(config, controllers);
    SnapshotCollector collector;
    engine.addObserver(&collector);
    engine.reset(controllers);
    collector.waitForFirstSnapshot();

    engine.start();

    collector.waitForGameOver();
}
