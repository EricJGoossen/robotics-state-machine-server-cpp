#pragma once

#include <optional>
#include <vector>

#include "territorygame/api/GridPosition.hpp"
#include "territorygame/api/VisibleCell.hpp"

namespace territorygame::helpers {

// A candidate-side memory of previously observed cells. Stores only the
// latest full VisibleCell (both layers) seen for each cell; never infers
// changes to cells that haven't been re-observed.
class ObservedBoard {
public:
    ObservedBoard(int width, int height);

    void update(const std::vector<std::vector<territorygame::api::VisibleCell>>& visibleGrid);
    std::optional<territorygame::api::VisibleCell> get(territorygame::api::GridPosition position) const;
    bool hasObserved(territorygame::api::GridPosition position) const;
    void clear();

private:
    int width_;
    int height_;
    std::vector<std::vector<std::optional<territorygame::api::VisibleCell>>> observed_;
};

} // namespace territorygame::helpers
