#include "territorygame/helpers/ObservedBoard.hpp"

#include "territorygame/helpers/MovementUtils.hpp"

namespace territorygame::helpers {

using territorygame::api::CellViewType;
using territorygame::api::GridPosition;
using territorygame::api::VisibleCell;

ObservedBoard::ObservedBoard(int width, int height)
    : width_(width), height_(height),
      observed_(height, std::vector<std::optional<CellViewType>>(width)) {}

void ObservedBoard::update(const std::vector<std::vector<VisibleCell>>& visibleGrid) {
    for (const auto& row : visibleGrid) {
        for (const auto& cell : row) {
            observed_[cell.position.y][cell.position.x] = cell.type;
        }
    }
}

std::optional<CellViewType> ObservedBoard::get(GridPosition position) const {
    if (!MovementUtils::isWithinBoard(position, width_, height_)) {
        return std::nullopt;
    }
    return observed_[position.y][position.x];
}

bool ObservedBoard::hasObserved(GridPosition position) const {
    return MovementUtils::isWithinBoard(position, width_, height_)
        && observed_[position.y][position.x].has_value();
}

void ObservedBoard::clear() {
    for (auto& row : observed_) {
        for (auto& cell : row) {
            cell.reset();
        }
    }
}

} // namespace territorygame::helpers
