#include "territorygame/domain/GameConfig.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "territorygame/helpers/MovementUtils.hpp"

namespace territorygame::domain {

using territorygame::api::GridPosition;

namespace {

constexpr const char* DEFAULT_RESOURCE = "src/main/resources/game-config.properties";

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::unordered_map<std::string, std::string> loadProperties(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Config resource not found: " + path);
    }
    std::unordered_map<std::string, std::string> properties;
    std::string line;
    while (std::getline(in, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }
        size_t eq = trimmed.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        properties[trim(trimmed.substr(0, eq))] = trim(trimmed.substr(eq + 1));
    }
    return properties;
}

int requireInt(const std::unordered_map<std::string, std::string>& properties, const std::string& key) {
    auto it = properties.find(key);
    if (it == properties.end()) {
        throw std::runtime_error("Missing required config key: " + key);
    }
    return std::stoi(it->second);
}

int64_t requireLong(const std::unordered_map<std::string, std::string>& properties, const std::string& key) {
    auto it = properties.find(key);
    if (it == properties.end()) {
        throw std::runtime_error("Missing required config key: " + key);
    }
    return std::stoll(it->second);
}

bool startingTerritoriesOverlap(
    const std::vector<GridPosition>& respawnPositions, int size, int width, int height) {
    std::unordered_set<GridPosition> seen;
    for (const auto& respawnPosition : respawnPositions) {
        for (const auto& cell : GameConfig::startingTerritoryAround(respawnPosition, size, width, height)) {
            if (!seen.insert(cell).second) {
                return true;
            }
        }
    }
    return false;
}

GameConfig validate(GameConfig config) {
    if (config.boardWidth <= 0 || config.boardHeight <= 0) {
        throw std::invalid_argument("board.width and board.height must be positive");
    }
    if (config.visibilityWindowSize <= 0 || config.visibilityWindowSize % 2 == 0) {
        throw std::invalid_argument("visibility.windowSize must be a positive odd number");
    }
    if (config.visibilityWindowSize > config.boardWidth || config.visibilityWindowSize > config.boardHeight) {
        throw std::invalid_argument("visibility.windowSize must not exceed the board dimensions");
    }
    if (config.turnsPerPlayer <= 0) {
        throw std::invalid_argument("turns.perPlayer must be positive");
    }
    if (config.startingTerritorySize <= 0) {
        throw std::invalid_argument("starting.territorySize must be positive");
    }
    if (config.autoPlayTurnDelayMillis < 0) {
        throw std::invalid_argument("autoplay.turnDelayMillis must not be negative");
    }
    if (config.maxAttemptsPerTurn <= 0) {
        throw std::invalid_argument("turn.maxAttemptsPerTurn must be positive");
    }
    for (const auto& position : config.respawnPositions) {
        if (!territorygame::helpers::MovementUtils::isWithinBoard(
                position, config.boardWidth, config.boardHeight)) {
            throw std::invalid_argument("Respawn position out of bounds");
        }
    }
    if (startingTerritoriesOverlap(
            config.respawnPositions, config.startingTerritorySize, config.boardWidth, config.boardHeight)) {
        throw std::invalid_argument("Starting territories overlap for the configured respawn positions");
    }
    if (config.controllerSeeds.size() != config.respawnPositions.size()) {
        throw std::invalid_argument("controllerSeeds must have exactly one entry per player");
    }
    return config;
}

} // namespace

std::vector<GridPosition> GameConfig::startingTerritoryAround(GridPosition center, int size, int width, int height) {
    int half = size / 2;
    std::vector<GridPosition> cells;
    for (int y = center.y - half; y <= center.y + half; y++) {
        for (int x = center.x - half; x <= center.x + half; x++) {
            GridPosition cell{x, y};
            if (territorygame::helpers::MovementUtils::isWithinBoard(cell, width, height)) {
                cells.push_back(cell);
            }
        }
    }
    return cells;
}

GameConfig GameConfig::loadFromFile(const std::string& path) {
    auto properties = loadProperties(path);
    int respawnCount = requireInt(properties, "respawn.count");

    std::vector<GridPosition> respawnPositions;
    std::vector<int64_t> controllerSeeds;
    respawnPositions.reserve(respawnCount);
    controllerSeeds.reserve(respawnCount);
    for (int i = 0; i < respawnCount; i++) {
        respawnPositions.push_back(GridPosition{
            requireInt(properties, "respawn." + std::to_string(i) + ".x"),
            requireInt(properties, "respawn." + std::to_string(i) + ".y")});
        controllerSeeds.push_back(requireLong(properties, "controller.seed." + std::to_string(i)));
    }

    GameConfig config{
        requireInt(properties, "board.width"),
        requireInt(properties, "board.height"),
        requireInt(properties, "visibility.windowSize"),
        requireInt(properties, "turns.perPlayer"),
        respawnPositions,
        requireInt(properties, "starting.territorySize"),
        requireInt(properties, "autoplay.turnDelayMillis"),
        requireInt(properties, "turn.maxAttemptsPerTurn"),
        controllerSeeds};
    return validate(config);
}

GameConfig GameConfig::loadDefault() { return loadFromFile(DEFAULT_RESOURCE); }

GameConfig GameConfig::create(
    int boardWidth, int boardHeight, int visibilityWindowSize, int turnsPerPlayer,
    std::vector<GridPosition> respawnPositions, int startingTerritorySize, int autoPlayTurnDelayMillis,
    int maxAttemptsPerTurn, std::vector<int64_t> controllerSeeds) {
    GameConfig config{
        boardWidth, boardHeight, visibilityWindowSize, turnsPerPlayer,
        std::move(respawnPositions), startingTerritorySize, autoPlayTurnDelayMillis,
        maxAttemptsPerTurn, std::move(controllerSeeds)};
    return validate(config);
}

} // namespace territorygame::domain
