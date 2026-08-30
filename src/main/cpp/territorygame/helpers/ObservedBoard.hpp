#pragma once

#include <optional>
#include <vector>

#include "territorygame/api/CellViewType.hpp"
#include "territorygame/api/GridPosition.hpp"
#include "territorygame/api/VisibleCell.hpp"

namespace territorygame::helpers {

// A candidate-side memory of previously observed cells. Stores only the
// latest value seen for each cell; never infers changes to cells that
// haven't been re-observed.
class ObservedBoard {
public:
    ObservedBoard(int width, int height);

    void update(const std::vector<std::vector<territorygame::api::VisibleCell>>& visibleGrid);
    std::optional<territorygame::api::CellViewType> get(territorygame::api::GridPosition position) const;
    bool hasObserved(territorygame::api::GridPosition position) const;
    void clear();

private:
    int width_;
    int height_;
    std::vector<std::vector<std::optional<territorygame::api::CellViewType>>> observed_;
};

} // namespace territorygame::helpers
