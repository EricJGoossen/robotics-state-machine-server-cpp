#pragma once

#include <vector>

#include "territorygame/engine/GameSnapshot.hpp"

namespace territorygame::gui {

// Cursor over snapshots the GUI has already received. Recording always
// appends and jumps to live; back/forward only change which snapshot is
// shown, never match state. Deliberately independent of ImGui so it (and
// ingestEngineSnapshot below) can be unit-tested without the GUI target's
// SDL2/Dear ImGui dependency.
template <typename T>
class SnapshotHistory {
public:
    void record(const T& snapshot) {
        snapshots_.push_back(snapshot);
        index_ = static_cast<int>(snapshots_.size()) - 1;
    }

    void clear() {
        snapshots_.clear();
        index_ = -1;
    }

    bool back() { return back(1); }

    bool back(int steps) {
        bool moved = false;
        for (int i = 0; i < steps; i++) {
            if (index_ <= 0) {
                return moved;
            }
            index_--;
            moved = true;
        }
        return moved;
    }

    bool forward() { return forward(1); }

    bool forward(int steps) {
        bool moved = false;
        for (int i = 0; i < steps; i++) {
            if (index_ >= static_cast<int>(snapshots_.size()) - 1) {
                return moved;
            }
            index_++;
            moved = true;
        }
        return moved;
    }

    const T& current() const { return snapshots_[static_cast<size_t>(index_)]; }

    bool canGoBack() const { return index_ > 0; }

    bool canGoForward() const { return index_ >= 0 && index_ < static_cast<int>(snapshots_.size()) - 1; }

    bool isAtLive() const { return index_ == static_cast<int>(snapshots_.size()) - 1; }

    // One-based, for status text ("Reviewing 3 / 10").
    int position() const { return index_ + 1; }

    int size() const { return static_cast<int>(snapshots_.size()); }

private:
    std::vector<T> snapshots_;
    int index_ = -1;
};

// Fresh matches (Reset / first paint) publish no lastMoveResult. Replace
// history so an in-flight Step that finished just before Reset cannot leave
// a leftover frame behind the new initial board.
inline void ingestEngineSnapshot(
    SnapshotHistory<territorygame::engine::GameSnapshot>& history,
    const territorygame::engine::GameSnapshot& snapshot) {
    if (!snapshot.lastMoveResult.has_value()) {
        history.clear();
    }
    history.record(snapshot);
}

} // namespace territorygame::gui
