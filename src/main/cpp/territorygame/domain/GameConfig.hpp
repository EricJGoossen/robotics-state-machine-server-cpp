#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "territorygame/api/GridPosition.hpp"

namespace territorygame::domain {

// Match configuration. Player count is implicit in respawnPositions.size().
// Window sizes are full odd side lengths (e.g. 11 for an 11x11 visibility
// window), not radii.
//
// Values are not hardcoded here; loadDefault() reads them from
// game-config.properties (the C++ port takes a filesystem path rather than
// a JVM classpath resource), so board size, visibility, turn count, respawn
// positions, and starting-territory shape can all be changed without
// touching code.
//
// Values are validated by loadFromFile()/loadDefault() so a misconfiguration
// fails immediately with a clear message instead of surfacing later as a
// confusing exception deep inside the engine.
struct GameConfig {
    int boardWidth;
    int boardHeight;
    int visibilityWindowSize;
    int turnsPerPlayer;
    std::vector<territorygame::api::GridPosition> respawnPositions;
    int startingTerritorySize;
    int autoPlayTurnDelayMillis;
    int maxAttemptsPerTurn;
    std::vector<int64_t> controllerSeeds;

    static GameConfig loadDefault();
    static GameConfig loadFromFile(const std::string& path);

    // Builds a config directly from field values, running the same
    // validation loadFromFile() does. Mirrors the Java record's public
    // constructor (which validates in a compact constructor run on every
    // construction path); useful for tests and for constructing configs
    // without a properties file.
    static GameConfig create(
        int boardWidth, int boardHeight, int visibilityWindowSize, int turnsPerPlayer,
        std::vector<territorygame::api::GridPosition> respawnPositions, int startingTerritorySize,
        int autoPlayTurnDelayMillis, int maxAttemptsPerTurn, std::vector<int64_t> controllerSeeds);

    // The square of cells (clipped to the board) centered on a respawn
    // position. Shared by validation and match setup.
    static std::vector<territorygame::api::GridPosition> startingTerritoryAround(
        territorygame::api::GridPosition center, int size, int width, int height);
};

} // namespace territorygame::domain
