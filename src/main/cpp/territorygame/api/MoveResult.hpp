#pragma once

namespace territorygame::api {

// Outcome of a single GameApi::move() call.
enum class MoveResult {
    MOVED,
    CAPTURED,
    DIED,
    INVALID
};

} // namespace territorygame::api
