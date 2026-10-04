#include "territorygame/engine/GameEngine.hpp"

#include <chrono>
#include <iostream>
#include <thread>

namespace territorygame::engine {

using namespace territorygame::api;
using namespace territorygame::domain;
using territorygame::rules::MoveResolver;
using territorygame::rules::RespawnService;
using territorygame::rules::TerritoryResolver;
using territorygame::visibility::VisibilityService;

GameEngine::GameEngine(
    GameConfig config, std::vector<std::shared_ptr<AgentController>> controllersInPlayerOrder)
    : config_(std::move(config)),
      controllersInPlayerOrder_(std::move(controllersInPlayerOrder)),
      turnManager_(config_.maxAttemptsPerTurn),
      turnDelayMillis_(config_.autoPlayTurnDelayMillis) {
    buildFreshMatch();
    worker_ = std::thread(&GameEngine::workerLoop, this);
}

GameEngine::~GameEngine() {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        stopping_ = true;
    }
    queueCv_.notify_all();
    if (worker_.joinable()) {
        worker_.join();
    }
}

void GameEngine::addObserver(GameObserver* observer) { observers_.push_back(observer); }

void GameEngine::setTurnDelayMillis(int turnDelayMillis) { turnDelayMillis_ = turnDelayMillis; }

void GameEngine::setController(int playerIndex, std::shared_ptr<AgentController> controller) {
    submitSafely([this, playerIndex, controller] {
        controllersInPlayerOrder_[static_cast<size_t>(playerIndex)] = controller;
        PlayerId id = state_->getPlayers()[static_cast<size_t>(playerIndex)].getId();
        controllers_[id] = controller.get();
    });
}

void GameEngine::start() {
    if (running_ || halted_) {
        return;
    }
    running_ = true;
    submitSafely([this] { runUntilPausedOrOver(); });
}

void GameEngine::pause() { running_ = false; }

void GameEngine::step() {
    submitSafely([this] {
        if (!halted_ && !state_->isGameOver()) {
            runSingleTurnAndPublish();
        }
    });
}

void GameEngine::reset() { reset(controllersInPlayerOrder_); }

void GameEngine::reset(std::vector<std::shared_ptr<AgentController>> controllersInPlayerOrder) {
    submitSafely([this, controllersInPlayerOrder = std::move(controllersInPlayerOrder)]() mutable {
        running_ = false;
        halted_ = false;
        haltMessage_.reset();
        controllersInPlayerOrder_ = std::move(controllersInPlayerOrder);
        buildFreshMatch();
        publish(buildSnapshot());
    });
}

void GameEngine::runUntilPausedOrOver() {
    while (running_ && !state_->isGameOver()) {
        runSingleTurnAndPublish();
        int delay = turnDelayMillis_;
        if (running_ && delay > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }
    running_ = false;
}

void GameEngine::submitSafely(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        tasks_.push_back(std::move(task));
    }
    queueCv_.notify_one();
}

void GameEngine::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCv_.wait(lock, [this] { return stopping_ || !tasks_.empty(); });
            if (tasks_.empty()) {
                if (stopping_) {
                    return;
                }
                continue;
            }
            task = std::move(tasks_.front());
            tasks_.pop_front();
        }
        try {
            task();
        } catch (const std::exception& e) {
            handleFatalError(e);
        }
    }
}

void GameEngine::handleFatalError(const std::exception& e) {
    running_ = false;
    halted_ = true;
    haltMessage_ = std::string("Internal engine error: ") + e.what();
    std::cerr << *haltMessage_ << std::endl;
    try {
        publish(buildSnapshot());
    } catch (...) {
        // buildSnapshot()/publish() itself is what's broken; nothing more we can safely do.
    }
}

void GameEngine::runSingleTurnAndPublish() {
    turnManager_.executeTurn(*state_, controllers_, apisRaw_);
    auto error = state_->getLastTurnError();
    if (error.has_value()) {
        std::cerr << *error << std::endl;
    }
    publish(buildSnapshot());
}

void GameEngine::buildFreshMatch() {
    Board board(config_.boardWidth, config_.boardHeight);
    std::vector<Player> players;
    players.reserve(config_.respawnPositions.size());
    for (size_t i = 0; i < config_.respawnPositions.size(); i++) {
        PlayerId id{static_cast<int>(i)};
        GridPosition respawnPosition = config_.respawnPositions[i];
        auto startingTerritory = GameConfig::startingTerritoryAround(
            respawnPosition, config_.startingTerritorySize, config_.boardWidth, config_.boardHeight);
        for (const auto& cell : startingTerritory) {
            board.setTerritoryOwner(cell, id);
        }
        Agent agent(respawnPosition, respawnPosition);
        players.emplace_back(id, std::move(agent), startingTerritory);
    }

    state_ = std::make_unique<GameState>(std::move(board), std::move(players), config_.turnsPerPlayer);

    moveResolver_ = std::make_unique<MoveResolver>(RespawnService(), TerritoryResolver());
    visibilityService_ = std::make_unique<VisibilityService>(config_.visibilityWindowSize);

    controllers_.clear();
    apis_.clear();
    apisRaw_.clear();
    for (size_t i = 0; i < state_->getPlayers().size(); i++) {
        PlayerId id = state_->getPlayers()[i].getId();
        controllers_[id] = controllersInPlayerOrder_[i].get();
        auto api = std::make_unique<GameApiImpl>(*state_, id, *moveResolver_, *visibilityService_);
        apisRaw_[id] = api.get();
        apis_[id] = std::move(api);
    }
}

void GameEngine::publish(const GameSnapshot& snapshot) {
    for (auto* observer : observers_) {
        observer->onGameStateChanged(snapshot);
    }
}

GameSnapshot GameEngine::buildSnapshot() const {
    const Board& board = state_->getBoard();
    int width = board.getWidth();
    int height = board.getHeight();

    std::vector<std::vector<GameSnapshot::CellSnapshot>> cells(
        height, std::vector<GameSnapshot::CellSnapshot>(width));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            GridPosition position{x, y};
            cells[y][x] = GameSnapshot::CellSnapshot{
                board.territoryOwnerAt(position), board.trailOwnerAt(position)};
        }
    }

    std::vector<GameSnapshot::PlayerSnapshot> playerSnapshots;
    playerSnapshots.reserve(state_->getPlayers().size());
    for (const auto& player : state_->getPlayers()) {
        playerSnapshots.push_back(GameSnapshot::PlayerSnapshot{
            player.getId(),
            player.getAgent().getPosition(),
            board.territoryCount(player.getId()),
            state_->getKillCount(player.getId()),
            state_->getDeathCount(player.getId()),
            state_->getRemainingTurns(player.getId()),
            player.getAgent().getActiveTrail(),
            controllers_.at(player.getId())->getDebugState()});
    }

    std::optional<std::string> errorMessage = halted_ ? haltMessage_ : state_->getLastTurnError();
    return GameSnapshot{
        width, height, std::move(cells), std::move(playerSnapshots),
        state_->getActivePlayerId(), state_->getLastMoveResult(),
        config_.visibilityWindowSize, state_->isGameOver(), errorMessage};
}

} // namespace territorygame::engine
