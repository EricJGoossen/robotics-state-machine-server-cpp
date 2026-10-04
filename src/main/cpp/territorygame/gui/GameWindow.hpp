#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "territorygame/controller/AvailableControllers.hpp"
#include "territorygame/domain/GameConfig.hpp"
#include "territorygame/engine/GameEngine.hpp"
#include "territorygame/engine/GameObserver.hpp"
#include "territorygame/engine/GameSnapshot.hpp"
#include "territorygame/gui/BoardPanel.hpp"
#include "territorygame/gui/SnapshotHistory.hpp"

namespace territorygame::gui {

// Top-level ImGui viewer for the full authoritative game. Lets the user
// pick which controller occupies each player slot and drives Start/Pause/
// Back/Forward/Step/Reset. Back and Forward only change which
// already-received snapshot is shown (via snapshotHistory_); they never
// rewind match state. Contains no game-rule logic; every update arrives as
// an immutable GameSnapshot published from GameEngine's background thread
// and is picked up here under a mutex for the next render() call on the
// main thread.
class GameWindow final : public territorygame::engine::GameObserver {
public:
    explicit GameWindow(territorygame::domain::GameConfig config);

    // Draws one frame's worth of ImGui widgets. Must be called on the
    // thread owning the ImGui context (the main/render thread), once per
    // frame, between ImGui::NewFrame() and ImGui::Render().
    void render();

    // GameObserver: invoked from GameEngine's background worker thread.
    void onGameStateChanged(const territorygame::engine::GameSnapshot& snapshot) override;

private:
    using ControllerOption = territorygame::controller::ControllerOption;

    std::vector<std::shared_ptr<territorygame::api::AgentController>> currentSelections();
    std::shared_ptr<territorygame::api::AgentController> controllerFrom(int optionIndex, int playerIndex);

    void renderControls();
    void renderError(const territorygame::engine::GameSnapshot* snapshot);
    void renderBoard(const territorygame::engine::GameSnapshot* snapshot);
    void renderStatus(const territorygame::engine::GameSnapshot* snapshot);
    void renderPlayerCard(int index, const territorygame::engine::GameSnapshot::PlayerSnapshot& player, bool active);
    static std::string winnerText(const territorygame::engine::GameSnapshot& snapshot);
    static int indexOfActivePlayer(const territorygame::engine::GameSnapshot& snapshot);

    void reviewBack(int steps);
    void reviewForward(int steps);

    // Converts between the slider's 0..SPEED_SLIDER_MAX travel and the
    // actual 0..MAX_TURN_DELAY_MILLIS delay through a square curve, so the
    // fast end of the bar (the range actually worth watching) isn't
    // cramped into its left half.
    static int delayToSlider(int delayMillis);
    static int sliderToDelay(int sliderValue);

    territorygame::domain::GameConfig config_;
    BoardPanel boardPanel_;
    std::unique_ptr<territorygame::engine::GameEngine> engine_;
    SnapshotHistory<territorygame::engine::GameSnapshot> snapshotHistory_;

    int player0Selection_ = 0; // Basic State Machine
    int player1Selection_ = 1; // Enemy State Machine
    int speedSliderValue_;

    std::mutex snapshotMutex_;
    std::optional<territorygame::engine::GameSnapshot> latestSnapshot_;
    bool hasNewSnapshot_ = false;
};

} // namespace territorygame::gui
