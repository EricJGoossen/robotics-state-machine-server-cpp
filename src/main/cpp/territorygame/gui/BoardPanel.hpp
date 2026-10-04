#pragma once

#include "imgui.h"
#include "territorygame/engine/GameSnapshot.hpp"

namespace territorygame::gui {

// Renders the full authoritative board into an ImGui draw list (no ImGui
// widgets of its own, no per-cell components): territory by owner color,
// trails, agents, and every player's visibility window (always shown, in
// that player's color -- not just the currently active player's, so the
// boxes don't flicker on and off as the turn alternates). Pure rendering,
// no game rules.
class BoardPanel {
public:
    BoardPanel(int boardWidth, int boardHeight);

    // Preferred pixel size before any snapshot has arrived; painting always
    // uses the live snapshot's own dimensions once one is available.
    ImVec2 preferredSize() const;

    // Shared with GameWindow's status panel so player colors match the board.
    static const ImU32 TERRITORY_COLORS[2];
    static const ImU32 AGENT_COLORS[2];

    // Paints into availableSize starting at origin (top-left, in screen
    // coordinates). No-op if snapshot is null.
    void paint(
        ImDrawList* drawList, ImVec2 origin, ImVec2 availableSize,
        const territorygame::engine::GameSnapshot* snapshot) const;

private:
    void paintCells(
        ImDrawList* drawList, ImVec2 origin, int cellSize,
        const territorygame::engine::GameSnapshot& snapshot) const;
    void paintAgents(
        ImDrawList* drawList, ImVec2 origin, int cellSize,
        const territorygame::engine::GameSnapshot& snapshot) const;
    void paintVisibilityWindows(
        ImDrawList* drawList, ImVec2 origin, int cellSize,
        const territorygame::engine::GameSnapshot& snapshot) const;

    int boardWidth_;
    int boardHeight_;
};

} // namespace territorygame::gui
