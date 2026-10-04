#pragma once

#include "territorygame/engine/GameSnapshot.hpp"

namespace territorygame::engine {

// Notified after each turn with the latest match state. Never mutates it.
class GameObserver {
public:
    virtual ~GameObserver() = default;
    virtual void onGameStateChanged(const GameSnapshot& snapshot) = 0;
};

} // namespace territorygame::engine
