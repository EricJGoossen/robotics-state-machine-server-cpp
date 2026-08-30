#pragma once

namespace territorygame::api {

/**
 * How a cell appears to a viewing player. Precedence when multiple concepts
 * occupy one cell is AGENT &gt; TRAIL &gt; TERRITORY &gt; FREE.
 */
enum class CellViewType {
    FREE,
    SELF_TERRITORY,
    OPPONENT_TERRITORY,
    SELF_TRAIL,
    OPPONENT_TRAIL,
    SELF_AGENT,
    OPPONENT_AGENT
};

} // namespace territorygame::api
