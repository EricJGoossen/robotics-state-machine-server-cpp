#pragma once

#include <functional>

namespace territorygame::domain {

// Opaque identity for a participant. Never a stand-in for "player 1"/"player 2" naming.
struct PlayerId {
    int index;

    bool operator==(const PlayerId& other) const { return index == other.index; }
    bool operator!=(const PlayerId& other) const { return !(*this == other); }
};

} // namespace territorygame::domain

namespace std {
template <>
struct hash<territorygame::domain::PlayerId> {
    size_t operator()(const territorygame::domain::PlayerId& id) const noexcept {
        return std::hash<int>()(id.index);
    }
};
} // namespace std
