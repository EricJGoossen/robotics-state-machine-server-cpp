#include "territorygame/domain/GameState.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace territorygame::domain {

using territorygame::api::MoveResult;

GameState::GameState(Board board, std::vector<Player> players, int turnsPerPlayer)
    : board_(std::move(board)), players_(std::move(players)), activePlayerId_(players_.front().getId()) {
    for (const auto& player : players_) {
        remainingTurns_[player.getId()] = turnsPerPlayer;
        killCounts_[player.getId()] = 0;
        deathCounts_[player.getId()] = 0;
    }
}

Board& GameState::getBoard() { return board_; }
const Board& GameState::getBoard() const { return board_; }

std::vector<Player>& GameState::getPlayers() { return players_; }
const std::vector<Player>& GameState::getPlayers() const { return players_; }

Player& GameState::getPlayer(PlayerId id) {
    for (auto& player : players_) {
        if (player.getId() == id) {
            return player;
        }
    }
    throw std::invalid_argument("Unknown player");
}

const Player& GameState::getPlayer(PlayerId id) const {
    for (const auto& player : players_) {
        if (player.getId() == id) {
            return player;
        }
    }
    throw std::invalid_argument("Unknown player");
}

Player& GameState::getOpponent(PlayerId id) {
    for (auto& player : players_) {
        if (!(player.getId() == id)) {
            return player;
        }
    }
    throw std::invalid_argument("No opponent found");
}

const Player& GameState::getOpponent(PlayerId id) const {
    for (const auto& player : players_) {
        if (!(player.getId() == id)) {
            return player;
        }
    }
    throw std::invalid_argument("No opponent found");
}

PlayerId GameState::getActivePlayerId() const { return activePlayerId_; }

void GameState::setActivePlayerId(PlayerId id) { activePlayerId_ = id; }

int GameState::getRemainingTurns(PlayerId id) const { return remainingTurns_.at(id); }

void GameState::decrementRemainingTurns(PlayerId id) { remainingTurns_[id] -= 1; }

bool GameState::isGameOver() const {
    return std::all_of(remainingTurns_.begin(), remainingTurns_.end(),
                        [](const auto& entry) { return entry.second <= 0; });
}

std::optional<MoveResult> GameState::getLastMoveResult() const { return lastMoveResult_; }

void GameState::setLastMoveResult(std::optional<MoveResult> result) { lastMoveResult_ = result; }

int GameState::getKillCount(PlayerId id) const { return killCounts_.at(id); }

void GameState::incrementKillCount(PlayerId id) { killCounts_[id] += 1; }

int GameState::getDeathCount(PlayerId id) const { return deathCounts_.at(id); }

void GameState::incrementDeathCount(PlayerId id) { deathCounts_[id] += 1; }

std::optional<std::string> GameState::getLastTurnError() const { return lastTurnError_; }

void GameState::setLastTurnError(std::optional<std::string> error) { lastTurnError_ = std::move(error); }

} // namespace territorygame::domain
