#include "territorygame/gui/BoardPanel.hpp"

#include <algorithm>

namespace territorygame::gui {

using territorygame::engine::GameSnapshot;

namespace {
constexpr ImU32 FREE_COLOR = IM_COL32(235, 235, 235, 255);
constexpr ImU32 GRID_LINE_COLOR = IM_COL32(210, 210, 210, 255);
constexpr ImU32 TRAIL_COLORS[2] = {IM_COL32(60, 110, 190, 255), IM_COL32(190, 80, 60, 255)};

// Used only to size the panel before the first snapshot arrives; painting
// always uses the live snapshot's dimensions.
constexpr int DEFAULT_CELL_SIZE = 14;
} // namespace

const ImU32 BoardPanel::TERRITORY_COLORS[2] = {IM_COL32(120, 170, 235, 255), IM_COL32(235, 140, 120, 255)};
const ImU32 BoardPanel::AGENT_COLORS[2] = {IM_COL32(20, 60, 130, 255), IM_COL32(140, 30, 20, 255)};

BoardPanel::BoardPanel(int boardWidth, int boardHeight) : boardWidth_(boardWidth), boardHeight_(boardHeight) {}

ImVec2 BoardPanel::preferredSize() const {
    return ImVec2(
        static_cast<float>(boardWidth_ * DEFAULT_CELL_SIZE), static_cast<float>(boardHeight_ * DEFAULT_CELL_SIZE));
}

void BoardPanel::paint(
    ImDrawList* drawList, ImVec2 origin, ImVec2 availableSize, const GameSnapshot* snapshot) const {
    if (snapshot == nullptr) {
        return;
    }
    int cellSize = std::max(
        1, std::min(
               static_cast<int>(availableSize.x) / snapshot->width,
               static_cast<int>(availableSize.y) / snapshot->height));
    // Centers the board within whatever region it's given, rather than
    // pinning it to the top-left, so extra space splits evenly on both sides.
    float centeredX = origin.x + (availableSize.x - cellSize * snapshot->width) / 2.0f;
    float centeredY = origin.y + (availableSize.y - cellSize * snapshot->height) / 2.0f;
    ImVec2 centeredOrigin(centeredX, centeredY);

    paintCells(drawList, centeredOrigin, cellSize, *snapshot);
    paintAgents(drawList, centeredOrigin, cellSize, *snapshot);
    paintVisibilityWindows(drawList, centeredOrigin, cellSize, *snapshot);
}

void BoardPanel::paintCells(
    ImDrawList* drawList, ImVec2 origin, int cellSize, const GameSnapshot& snapshot) const {
    // Territory fill first, then trail as a smaller inset square, so a
    // trail cutting across owned land no longer hides which player's
    // territory lies underneath it.
    for (int y = 0; y < snapshot.height; y++) {
        for (int x = 0; x < snapshot.width; x++) {
            const GameSnapshot::CellSnapshot& cell = snapshot.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
            ImU32 territoryColor =
                cell.territoryOwner.has_value() ? TERRITORY_COLORS[cell.territoryOwner->index % 2] : FREE_COLOR;
            ImVec2 min(origin.x + x * cellSize, origin.y + y * cellSize);
            ImVec2 max(min.x + cellSize, min.y + cellSize);
            drawList->AddRectFilled(min, max, territoryColor);

            if (cell.trailOwner.has_value()) {
                int inset = std::max(1, cellSize / 4);
                ImVec2 trailMin(min.x + inset, min.y + inset);
                ImVec2 trailMax(max.x - inset, max.y - inset);
                drawList->AddRectFilled(trailMin, trailMax, TRAIL_COLORS[cell.trailOwner->index % 2]);
            }
        }
    }

    float boardWidthPx = static_cast<float>(snapshot.width * cellSize);
    float boardHeightPx = static_cast<float>(snapshot.height * cellSize);
    for (int x = 0; x <= snapshot.width; x++) {
        float px = origin.x + x * cellSize;
        drawList->AddLine(ImVec2(px, origin.y), ImVec2(px, origin.y + boardHeightPx), GRID_LINE_COLOR);
    }
    for (int y = 0; y <= snapshot.height; y++) {
        float py = origin.y + y * cellSize;
        drawList->AddLine(ImVec2(origin.x, py), ImVec2(origin.x + boardWidthPx, py), GRID_LINE_COLOR);
    }
}

void BoardPanel::paintAgents(
    ImDrawList* drawList, ImVec2 origin, int cellSize, const GameSnapshot& snapshot) const {
    for (const auto& player : snapshot.players) {
        ImU32 color = AGENT_COLORS[player.id.index % 2];
        int margin = std::max(1, cellSize / 6);
        float centerX = origin.x + player.position.x * cellSize + cellSize / 2.0f;
        float centerY = origin.y + player.position.y * cellSize + cellSize / 2.0f;
        float radius = cellSize / 2.0f - margin;
        drawList->AddCircleFilled(ImVec2(centerX, centerY), std::max(1.0f, radius), color);
    }
}

void BoardPanel::paintVisibilityWindows(
    ImDrawList* drawList, ImVec2 origin, int cellSize, const GameSnapshot& snapshot) const {
    int half = snapshot.visibilityWindowSize / 2;
    for (const auto& player : snapshot.players) {
        ImU32 color = AGENT_COLORS[player.id.index % 2];
        int minX = std::max(0, player.position.x - half);
        int minY = std::max(0, player.position.y - half);
        int maxX = std::min(snapshot.width - 1, player.position.x + half);
        int maxY = std::min(snapshot.height - 1, player.position.y + half);

        ImVec2 min(origin.x + minX * cellSize, origin.y + minY * cellSize);
        ImVec2 max(origin.x + (maxX + 1) * cellSize, origin.y + (maxY + 1) * cellSize);
        drawList->AddRect(min, max, color);
    }
}

} // namespace territorygame::gui
