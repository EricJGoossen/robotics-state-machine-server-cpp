// Headless console entry point for the C++ port. The Java original launches
// a Swing GUI (see GuiMain.cpp for this port's SDL2/ImGui equivalent); this
// binary instead runs one match to completion on GameEngine's background
// thread and prints periodic progress plus the final result to stdout, for
// use in scripts/CI where a window isn't wanted.

#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

#include "territorygame/controller/AvailableControllers.hpp"
#include "territorygame/domain/GameConfig.hpp"
#include "territorygame/engine/GameEngine.hpp"
#include "territorygame/engine/GameObserver.hpp"
#include "territorygame/engine/GameSnapshot.hpp"

namespace {

using territorygame::controller::availableControllers;
using territorygame::domain::GameConfig;
using territorygame::engine::GameEngine;
using territorygame::engine::GameObserver;
using territorygame::engine::GameSnapshot;

// GameEngine publishes from its own background thread; this observer
// prints as those arrive and lets main() block until the match is actually
// over instead of returning as soon as start() has merely been submitted.
class ConsoleObserver final : public GameObserver {
public:
    explicit ConsoleObserver(int printEveryNTurns) : printEveryNTurns_(printEveryNTurns) {}

    void onGameStateChanged(const GameSnapshot& snapshot) override {
        turnsSeen_++;
        if (snapshot.errorMessage.has_value()) {
            std::cerr << *snapshot.errorMessage << std::endl;
        }
        bool print = snapshot.gameOver || turnsSeen_ % printEveryNTurns_ == 0;
        if (print) {
            printSummary(snapshot);
        }
        if (snapshot.gameOver) {
            std::lock_guard<std::mutex> lock(mutex_);
            gameOver_ = true;
            cv_.notify_all();
        }
    }

    // Blocks the calling thread until a game-over snapshot has been observed.
    void waitForGameOver() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return gameOver_; });
    }

private:
    void printSummary(const GameSnapshot& snapshot) const {
        std::cout << "-- turn " << turnsSeen_ << (snapshot.gameOver ? " (final)" : "") << " --" << std::endl;
        for (const auto& player : snapshot.players) {
            std::cout << "  player " << player.id.index
                       << " pos=(" << player.position.x << "," << player.position.y << ")"
                       << " territory=" << player.territoryCount
                       << " kills=" << player.killCount
                       << " deaths=" << player.deathCount
                       << " turnsLeft=" << player.remainingTurns
                       << " score=" << player.score();
            if (player.debugState.has_value()) {
                std::cout << " state=" << *player.debugState;
            }
            std::cout << std::endl;
        }
    }

    int printEveryNTurns_;
    int turnsSeen_ = 0;

    std::mutex mutex_;
    std::condition_variable cv_;
    bool gameOver_ = false;
};

} // namespace

int main() {
    GameConfig config = GameConfig::loadDefault();
    const auto& options = availableControllers();

    // Enemy State Machine (framework's standard opponent) vs. Candidate
    // Controller (the file a candidate edits), matching the two configured
    // respawn slots.
    std::vector<std::shared_ptr<territorygame::api::AgentController>> controllers{
        options[1].factory(config.controllerSeeds[0]), // Enemy State Machine
        options[3].factory(config.controllerSeeds[1]), // Candidate Controller
    };

    GameEngine engine(config, controllers);
    // Headless run: skip the configured autoplay pacing delay (meant for a
    // human watching a GUI) so the match finishes immediately.
    engine.setTurnDelayMillis(0);
    ConsoleObserver observer(/*printEveryNTurns=*/200);
    engine.addObserver(&observer);

    std::cout << "Territory Capture (headless) -- board " << config.boardWidth << "x" << config.boardHeight
               << ", " << config.turnsPerPlayer << " turns/player" << std::endl;

    engine.start();
    observer.waitForGameOver();

    std::cout << "Match complete." << std::endl;
    return 0;
}
