#pragma once

#include "territorygame/api/GridPosition.hpp"
#include "territorygame/api/OccupantView.hpp"
#include "territorygame/api/TerritoryView.hpp"

namespace territorygame::api {

// One cell of a GameApi::getVisibleGrid() window, with its absolute
// position. occupant is the head/trail layer; territory is the land
// underneath. Both are always present -- a trail does not hide territory.
struct VisibleCell {
    GridPosition position;
    OccupantView occupant;
    TerritoryView territory;
};

} // namespace territorygame::api
