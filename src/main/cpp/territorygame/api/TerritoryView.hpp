#pragma once

namespace territorygame::api {

/**
 * Territory layer of a cell as seen by a viewing player. Independent of
 * whether an agent or trail is also on the cell.
 */
enum class TerritoryView {
    UNOWNED,
    SELF,
    OPPONENT
};

} // namespace territorygame::api
