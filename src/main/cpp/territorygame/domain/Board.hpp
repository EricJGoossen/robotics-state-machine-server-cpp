#pragma once

#include <optional>
#include <unordered_set>
#include <vector>

#include "territorygame/api/GridPosition.hpp"
#include "territorygame/domain/PlayerId.hpp"

namespace territorygame::domain {

// Authoritative board-cell state. Agent positions are not tracked here; they
// live on Agent and are considered separately by callers doing occupancy
// checks.
class Board {
public:
    Board(int width, int height);

    int getWidth() const;
    int getHeight() const;
    bool isWithinBounds(territorygame::api::GridPosition position) const;

    std::optional<PlayerId> territoryOwnerAt(territorygame::api::GridPosition position) const;
    std::optional<PlayerId> trailOwnerAt(territorygame::api::GridPosition position) const;
    void setTerritoryOwner(territorygame::api::GridPosition position, std::optional<PlayerId> owner);
    void setTrailOwner(territorygame::api::GridPosition position, std::optional<PlayerId> owner);

    int territoryCount(PlayerId owner) const;
    std::unordered_set<territorygame::api::GridPosition> territoryOf(PlayerId owner) const;
    void clearAllTerritoryOf(PlayerId owner);

private:
    struct BoardCell {
        std::optional<PlayerId> territoryOwner;
        std::optional<PlayerId> trailOwner;
    };

    BoardCell& cellAt(territorygame::api::GridPosition position);
    const BoardCell& cellAt(territorygame::api::GridPosition position) const;

    int width_;
    int height_;
    std::vector<std::vector<BoardCell>> cells_; // cells_[y][x]
};

} // namespace territorygame::domain
