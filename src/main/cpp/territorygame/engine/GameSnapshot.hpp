#pragma once

#include <optional>
#include <string>
#include <vector>

#include "territorygame/api/GridPosition.hpp"
#include "territorygame/api/MoveResult.hpp"
#include "territorygame/domain/PlayerId.hpp"

namespace territorygame::engine {

// Immutable, fully copied view of match state for observers (the GUI, or a
// console printer). Contains no behavior beyond display-derived values.
//
// errorMessage is set exactly when the most recent turn (or the engine
// itself) failed in some way worth surfacing to the user; it is
// display-only and never affects match state.
struct GameSnapshot {
    // Territory/trail ownership of one cell; either owner may be absent.
    struct CellSnapshot {
        std::optional<territorygame::domain::PlayerId> territoryOwner;
        std::optional<territorygame::domain::PlayerId> trailOwner;

        bool operator==(const CellSnapshot& other) const {
            return territoryOwner == other.territoryOwner && trailOwner == other.trailOwner;
        }
        bool operator!=(const CellSnapshot& other) const { return !(*this == other); }
    };

    // One player's displayable state at the moment of the snapshot.
    struct PlayerSnapshot {
        static constexpr int KILL_SCORE_BONUS = 10;

        territorygame::domain::PlayerId id;
        territorygame::api::GridPosition position;
        int territoryCount;
        int killCount;
        int deathCount;
        int remainingTurns;
        std::vector<territorygame::api::GridPosition> trail;
        std::optional<std::string> debugState;

        // Display-only composite score; the win condition still uses territoryCount alone.
        int score() const { return territoryCount + killCount * KILL_SCORE_BONUS; }

        bool operator==(const PlayerSnapshot& other) const {
            return id == other.id && position == other.position && territoryCount == other.territoryCount
                && killCount == other.killCount && deathCount == other.deathCount
                && remainingTurns == other.remainingTurns && trail == other.trail
                && debugState == other.debugState;
        }
        bool operator!=(const PlayerSnapshot& other) const { return !(*this == other); }
    };

    int width;
    int height;
    std::vector<std::vector<CellSnapshot>> cells;
    std::vector<PlayerSnapshot> players;
    territorygame::domain::PlayerId activePlayerId;
    std::optional<territorygame::api::MoveResult> lastMoveResult;
    int visibilityWindowSize;
    bool gameOver;
    std::optional<std::string> errorMessage;

    bool operator==(const GameSnapshot& other) const {
        return width == other.width && height == other.height && gameOver == other.gameOver
            && visibilityWindowSize == other.visibilityWindowSize && cells == other.cells
            && players == other.players && activePlayerId == other.activePlayerId
            && lastMoveResult == other.lastMoveResult && errorMessage == other.errorMessage;
    }
    bool operator!=(const GameSnapshot& other) const { return !(*this == other); }
};

} // namespace territorygame::engine
