#pragma once

// Test-only factory for small, deterministic two-player game states.

#include <vector>

#include "territorygame/api/GridPosition.hpp"
#include "territorygame/domain/Agent.hpp"
#include "territorygame/domain/Board.hpp"
#include "territorygame/domain/GameState.hpp"
#include "territorygame/domain/Player.hpp"
#include "territorygame/domain/PlayerId.hpp"

namespace territorygame::test {

inline territorygame::domain::GameState twoPlayerState(
    int width, int height,
    territorygame::api::GridPosition position0, std::vector<territorygame::api::GridPosition> territory0,
    territorygame::api::GridPosition position1, std::vector<territorygame::api::GridPosition> territory1,
    int turnsPerPlayer) {
    using territorygame::domain::Agent;
    using territorygame::domain::Board;
    using territorygame::domain::GameState;
    using territorygame::domain::Player;
    using territorygame::domain::PlayerId;

    Board board(width, height);
    PlayerId id0{0};
    PlayerId id1{1};

    Agent agent0(position0, position0);
    Agent agent1(position1, position1);

    for (const auto& cell : territory0) {
        board.setTerritoryOwner(cell, id0);
    }
    for (const auto& cell : territory1) {
        board.setTerritoryOwner(cell, id1);
    }

    std::vector<Player> players;
    players.emplace_back(id0, std::move(agent0), territory0);
    players.emplace_back(id1, std::move(agent1), territory1);

    return GameState(std::move(board), std::move(players), turnsPerPlayer);
}

} // namespace territorygame::test
