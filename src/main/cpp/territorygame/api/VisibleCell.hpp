#pragma once

#include "territorygame/api/CellViewType.hpp"
#include "territorygame/api/GridPosition.hpp"

namespace territorygame::api {

// One cell of a GameApi::getVisibleGrid() window, with its absolute position.
struct VisibleCell {
    GridPosition position;
    CellViewType type;
};

} // namespace territorygame::api
