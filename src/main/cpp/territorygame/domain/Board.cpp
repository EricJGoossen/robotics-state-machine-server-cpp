#include "territorygame/domain/Board.hpp"

#include <stdexcept>

namespace territorygame::domain {

using territorygame::api::GridPosition;

Board::Board(int width, int height)
    : width_(width), height_(height), cells_(height, std::vector<BoardCell>(width)) {}

int Board::getWidth() const { return width_; }

int Board::getHeight() const { return height_; }

bool Board::isWithinBounds(GridPosition position) const {
    return position.x >= 0 && position.x < width_ && position.y >= 0 && position.y < height_;
}

Board::BoardCell& Board::cellAt(GridPosition position) {
    if (!isWithinBounds(position)) {
        throw std::invalid_argument("Position out of bounds");
    }
    return cells_[position.y][position.x];
}

const Board::BoardCell& Board::cellAt(GridPosition position) const {
    if (!isWithinBounds(position)) {
        throw std::invalid_argument("Position out of bounds");
    }
    return cells_[position.y][position.x];
}

std::optional<PlayerId> Board::territoryOwnerAt(GridPosition position) const {
    return cellAt(position).territoryOwner;
}

std::optional<PlayerId> Board::trailOwnerAt(GridPosition position) const {
    return cellAt(position).trailOwner;
}

void Board::setTerritoryOwner(GridPosition position, std::optional<PlayerId> owner) {
    cellAt(position).territoryOwner = owner;
}

void Board::setTrailOwner(GridPosition position, std::optional<PlayerId> owner) {
    cellAt(position).trailOwner = owner;
}

int Board::territoryCount(PlayerId owner) const {
    int count = 0;
    for (int y = 0; y < height_; y++) {
        for (int x = 0; x < width_; x++) {
            if (cells_[y][x].territoryOwner == owner) {
                count++;
            }
        }
    }
    return count;
}

std::unordered_set<GridPosition> Board::territoryOf(PlayerId owner) const {
    std::unordered_set<GridPosition> positions;
    for (int y = 0; y < height_; y++) {
        for (int x = 0; x < width_; x++) {
            if (cells_[y][x].territoryOwner == owner) {
                positions.insert(GridPosition{x, y});
            }
        }
    }
    return positions;
}

void Board::clearAllTerritoryOf(PlayerId owner) {
    for (const auto& position : territoryOf(owner)) {
        setTerritoryOwner(position, std::nullopt);
    }
}

} // namespace territorygame::domain
