#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "territorygame/api/MoveResult.hpp"
#include "territorygame/domain/Board.hpp"
#include "territorygame/domain/Player.hpp"
#include "territorygame/domain/PlayerId.hpp"

namespace territorygame::domain {

// Authoritative mutable match state: board, players, per-player remaining
// turns, and whose turn it is. Never exposed to candidate or GUI code
// directly. Assumes exactly two players, matching every rule in the spec
// that refers to "the other agent" / "the opponent".
class GameState {
public:
    GameState(Board board, std::vector<Player> players, int turnsPerPlayer);

    Board& getBoard();
    const Board& getBoard() const;

    std::vector<Player>& getPlayers();
    const std::vector<Player>& getPlayers() const;

    Player& getPlayer(PlayerId id);
    const Player& getPlayer(PlayerId id) const;

    // The other participant in this two-player match.
    Player& getOpponent(PlayerId id);
    const Player& getOpponent(PlayerId id) const;

    PlayerId getActivePlayerId() const;
    void setActivePlayerId(PlayerId id);

    int getRemainingTurns(PlayerId id) const;
    void decrementRemainingTurns(PlayerId id);
    bool isGameOver() const;

    std::optional<territorygame::api::MoveResult> getLastMoveResult() const;
    void setLastMoveResult(std::optional<territorygame::api::MoveResult> result);

    int getKillCount(PlayerId id) const;
    void incrementKillCount(PlayerId id);
    int getDeathCount(PlayerId id) const;
    void incrementDeathCount(PlayerId id);

    // Display-only description of why the most recent turn produced no successful move, if any.
    std::optional<std::string> getLastTurnError() const;
    void setLastTurnError(std::optional<std::string> error);

private:
    Board board_;
    std::vector<Player> players_;
    std::unordered_map<PlayerId, int> remainingTurns_;
    std::unordered_map<PlayerId, int> killCounts_;
    std::unordered_map<PlayerId, int> deathCounts_;
    PlayerId activePlayerId_;
    std::optional<territorygame::api::MoveResult> lastMoveResult_;
    std::optional<std::string> lastTurnError_;
};

} // namespace territorygame::domain
