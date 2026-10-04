#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "territorygame/api/AgentController.hpp"
#include "territorygame/domain/GameConfig.hpp"
#include "territorygame/domain/GameState.hpp"
#include "territorygame/domain/PlayerId.hpp"
#include "territorygame/engine/GameApiImpl.hpp"
#include "territorygame/engine/GameObserver.hpp"
#include "territorygame/engine/GameSnapshot.hpp"
#include "territorygame/engine/TurnManager.hpp"
#include "territorygame/rules/MoveResolver.hpp"
#include "territorygame/visibility/VisibilityService.hpp"

namespace territorygame::engine {

// Coordinates a match: lifecycle, turn order, controller invocation, the
// end condition, and observer notification. Implements no capture logic,
// visibility generation, or rendering.
//
// The game is turn-based and single-player-active at a time, so all state
// mutation is confined to one background worker thread: every command
// (start/step/reset/setController) is a task pushed onto a queue that the
// worker drains in submission order, mirroring the Java original's
// single-thread ExecutorService. This keeps game execution off the GUI's
// render thread without any hand-written locking around game state itself.
// Call reset() once after registering observers to trigger the first
// render.
class GameEngine {
public:
    GameEngine(
        territorygame::domain::GameConfig config,
        std::vector<std::shared_ptr<territorygame::api::AgentController>> controllersInPlayerOrder);
    ~GameEngine();

    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;

    void addObserver(GameObserver* observer);

    // Changes the pause between turns during continuous play (see start()). Takes effect from the next turn on.
    void setTurnDelayMillis(int turnDelayMillis);

    // Replaces the controller for one player slot (0-based, matching the
    // order passed to the constructor/reset) without resetting the match —
    // position, territory, trail, and turns are all left as they are.
    void setController(int playerIndex, std::shared_ptr<territorygame::api::AgentController> controller);

    // Runs turns continuously until paused or the match ends. No-op if already running or halted.
    void start();

    // Stops the run loop after the in-flight turn finishes.
    void pause();

    // Runs exactly one turn, regardless of the running flag. No-op if halted or already over.
    void step();

    // Rebuilds a fresh match, keeping the current controllers.
    void reset();

    // Rebuilds a fresh match with a new set of controllers.
    void reset(std::vector<std::shared_ptr<territorygame::api::AgentController>> controllersInPlayerOrder);

private:
    void runUntilPausedOrOver();
    void runSingleTurnAndPublish();
    void buildFreshMatch();
    void publish(const GameSnapshot& snapshot);
    GameSnapshot buildSnapshot() const;
    void handleFatalError(const std::exception& e);

    // Pushes a task onto the worker queue, guarding against any exception
    // that isn't already contained by TurnManager (e.g. a bug in the
    // engine's own plumbing rather than a controller). Without this, such
    // an exception would kill the worker thread silently and the match
    // would just stop updating with no visible cause.
    void submitSafely(std::function<void()> task);
    void workerLoop();

    territorygame::domain::GameConfig config_;
    std::vector<std::shared_ptr<territorygame::api::AgentController>> controllersInPlayerOrder_;
    TurnManager turnManager_;
    std::vector<GameObserver*> observers_;

    std::unique_ptr<territorygame::domain::GameState> state_;
    std::unique_ptr<territorygame::rules::MoveResolver> moveResolver_;
    std::unique_ptr<territorygame::visibility::VisibilityService> visibilityService_;
    std::unordered_map<territorygame::domain::PlayerId, territorygame::api::AgentController*> controllers_;
    std::unordered_map<territorygame::domain::PlayerId, std::unique_ptr<GameApiImpl>> apis_;
    std::unordered_map<territorygame::domain::PlayerId, GameApiImpl*> apisRaw_;

    std::atomic<bool> running_{false};
    bool halted_ = false;
    std::optional<std::string> haltMessage_;
    std::atomic<int> turnDelayMillis_;

    // Single-thread executor: every command (start/step/reset/
    // setController) is a task pushed here and drained by worker_ in
    // submission order, which is what confines all game-state mutation to
    // one thread. pause() is the one exception — it just flips running_
    // directly from the caller's thread, matching the Java original.
    std::thread worker_;
    std::mutex queueMutex_;
    std::condition_variable queueCv_;
    std::deque<std::function<void()>> tasks_;
    bool stopping_ = false;
};

} // namespace territorygame::engine
