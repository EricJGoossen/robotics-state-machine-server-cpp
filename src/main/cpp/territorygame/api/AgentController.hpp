#pragma once

#include <optional>
#include <string>

#include "territorygame/api/GameApi.hpp"

namespace territorygame::api {

/**
 * Strategy a player supplies to play the game. The framework invokes
 * {@link #takeTurn(GameApi)} repeatedly for the same turn until a
 * successful move is made.
 */
class AgentController {
public:
    virtual ~AgentController() = default;

    virtual void takeTurn(GameApi& game) = 0;

    /**
     * Optional label for whatever internal state this controller considers
     * itself to be in right now (e.g. an enum name), shown next to it in the
     * GUI. Purely for observing a match; has no effect on gameplay. Return
     * {@code null} (the default) if there's nothing worth showing.
     */
    virtual std::optional<std::string> getDebugState() const { return std::nullopt; }
};

} // namespace territorygame::api
