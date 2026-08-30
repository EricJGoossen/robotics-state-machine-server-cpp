#include "territorygame/gui/GameWindow.hpp"

#include "imgui.h"

namespace territorygame::gui {

using territorygame::api::AgentController;
using territorygame::controller::availableControllers;
using territorygame::domain::GameConfig;
using territorygame::engine::GameEngine;
using territorygame::engine::GameSnapshot;

namespace {
constexpr int MIN_TURN_DELAY_MILLIS = 0;
constexpr int MAX_TURN_DELAY_MILLIS = 500;
} // namespace

GameWindow::GameWindow(GameConfig config)
    : config_(config),
      boardPanel_(config.boardWidth, config.boardHeight),
      turnDelayMillis_(config.autoPlayTurnDelayMillis) {
    auto initialControllers = currentSelections();
    engine_ = std::make_unique<GameEngine>(config_, initialControllers);
    engine_->addObserver(this);

    // Triggers the first observer notification so the board paints before
    // Start/Step is ever clicked; reuses the same controller instances just
    // constructed above rather than creating a second set.
    engine_->reset(initialControllers);
}

std::shared_ptr<AgentController> GameWindow::controllerFrom(int optionIndex, int playerIndex) {
    const auto& options = availableControllers();
    int64_t seed = config_.controllerSeeds[static_cast<size_t>(playerIndex)];
    return options[static_cast<size_t>(optionIndex)].factory(seed);
}

std::vector<std::shared_ptr<AgentController>> GameWindow::currentSelections() {
    return {controllerFrom(player0Selection_, 0), controllerFrom(player1Selection_, 1)};
}

void GameWindow::onGameStateChanged(const GameSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(snapshotMutex_);
    latestSnapshot_ = snapshot;
}

void GameWindow::render() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("Territory Capture", nullptr, flags);

    std::optional<GameSnapshot> snapshotCopy;
    {
        std::lock_guard<std::mutex> lock(snapshotMutex_);
        snapshotCopy = latestSnapshot_;
    }
    const GameSnapshot* snapshot = snapshotCopy.has_value() ? &(*snapshotCopy) : nullptr;

    renderControls();
    renderError(snapshot);
    renderBoard(snapshot);
    renderStatus(snapshot);

    ImGui::End();
}

void GameWindow::renderControls() {
    const auto& options = availableControllers();

    ImGui::Text("Player 1:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    if (ImGui::BeginCombo("##player0", options[static_cast<size_t>(player0Selection_)].label.c_str())) {
        for (int i = 0; i < static_cast<int>(options.size()); i++) {
            bool selected = i == player0Selection_;
            if (ImGui::Selectable(options[static_cast<size_t>(i)].label.c_str(), selected)) {
                player0Selection_ = i;
                // Swaps the controller in for that slot from the next turn
                // on, without resetting the match -- position, territory,
                // trail, and turns are all left as they are.
                engine_->setController(0, controllerFrom(player0Selection_, 0));
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine(0, 20);
    ImGui::Text("Player 2:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    if (ImGui::BeginCombo("##player1", options[static_cast<size_t>(player1Selection_)].label.c_str())) {
        for (int i = 0; i < static_cast<int>(options.size()); i++) {
            bool selected = i == player1Selection_;
            if (ImGui::Selectable(options[static_cast<size_t>(i)].label.c_str(), selected)) {
                player1Selection_ = i;
                engine_->setController(1, controllerFrom(player1Selection_, 1));
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Start")) {
        engine_->start();
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        engine_->pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Step")) {
        engine_->step();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        engine_->reset(currentSelections());
    }

    ImGui::SameLine(0, 28);
    ImGui::Text("Speed:");
    ImGui::SameLine();
    ImGui::Text("Fast");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    // Slider value is the turn delay in milliseconds: left (fast) is 0, right (slow) is the max.
    if (ImGui::SliderInt("##speed", &turnDelayMillis_, MIN_TURN_DELAY_MILLIS, MAX_TURN_DELAY_MILLIS, "")) {
        engine_->setTurnDelayMillis(turnDelayMillis_);
    }
    ImGui::SameLine();
    ImGui::Text("Slow");

    ImGui::Separator();
}

void GameWindow::renderError(const GameSnapshot* snapshot) {
    if (snapshot == nullptr || !snapshot->errorMessage.has_value()) {
        return;
    }
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 0, 0, 255));
    ImGui::TextWrapped("%s", snapshot->errorMessage->c_str());
    ImGui::PopStyleColor();
    ImGui::Separator();
}

void GameWindow::renderBoard(const GameSnapshot* snapshot) {
    ImVec2 preferred = boardPanel_.preferredSize();
    // Reserve space for the status panel below so the board doesn't crowd it out.
    float statusHeight = 190.0f;
    ImVec2 available = ImGui::GetContentRegionAvail();
    ImVec2 boardRegion(available.x, std::max(preferred.y, available.y - statusHeight));

    ImGui::BeginChild("BoardPanel", boardRegion, true);
    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    boardPanel_.paint(drawList, origin, canvasSize, snapshot);
    ImGui::EndChild();
}

void GameWindow::renderStatus(const GameSnapshot* snapshot) {
    ImGui::BeginChild("StatusPanel", ImVec2(0, 0), false);
    if (snapshot != nullptr) {
        for (size_t i = 0; i < snapshot->players.size() && i < 2; i++) {
            const auto& player = snapshot->players[i];
            bool active = player.id == snapshot->activePlayerId;
            renderPlayerCard(static_cast<int>(i), player, active);
            if (i == 0) {
                ImGui::SameLine();
            }
        }

        std::string statusText;
        if (snapshot->gameOver) {
            statusText = "Game over -- " + winnerText(*snapshot);
        } else {
            statusText = "Active: Player " + std::to_string(indexOfActivePlayer(*snapshot) + 1);
            statusText += "   |   Last move: ";
            if (snapshot->lastMoveResult.has_value()) {
                switch (*snapshot->lastMoveResult) {
                    case territorygame::api::MoveResult::MOVED: statusText += "MOVED"; break;
                    case territorygame::api::MoveResult::CAPTURED: statusText += "CAPTURED"; break;
                    case territorygame::api::MoveResult::DIED: statusText += "DIED"; break;
                    case territorygame::api::MoveResult::INVALID: statusText += "INVALID"; break;
                }
            } else {
                statusText += "-";
            }
        }
        ImGui::Separator();
        float textWidth = ImGui::CalcTextSize(statusText.c_str()).x;
        ImGui::SetCursorPosX(std::max(0.0f, (ImGui::GetContentRegionAvail().x - textWidth) / 2.0f));
        ImGui::TextUnformatted(statusText.c_str());
    }
    ImGui::EndChild();
}

void GameWindow::renderPlayerCard(int index, const GameSnapshot::PlayerSnapshot& player, bool active) {
    ImGui::BeginChild(
        index == 0 ? "Player1Card" : "Player2Card", ImVec2(ImGui::GetContentRegionAvail().x / (index == 0 ? 2 : 1), 180),
        true);

    std::string title = "Player " + std::to_string(player.id.index + 1) + (active ? " (active)" : "");
    ImGui::TextColored(ImVec4(0, 0, 0, 1), "%s", title.c_str());
    ImGui::Separator();

    ImU32 swatchColor = BoardPanel::AGENT_COLORS[player.id.index % 2];
    ImVec2 swatchPos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(
        swatchPos, ImVec2(swatchPos.x + 13, swatchPos.y + 13), swatchColor);
    ImGui::Dummy(ImVec2(16, 13));
    ImGui::SameLine();
    ImGui::Text("Score: %d", player.score());

    ImGui::Text("Territory: %d", player.territoryCount);
    ImGui::Text("Kills: %d", player.killCount);
    ImGui::Text("Deaths: %d", player.deathCount);
    ImGui::Text("Turns left: %d", player.remainingTurns);
    ImGui::Text("State: %s", player.debugState.has_value() ? player.debugState->c_str() : " ");

    ImGui::EndChild();
}

int GameWindow::indexOfActivePlayer(const GameSnapshot& snapshot) {
    for (size_t i = 0; i < snapshot.players.size(); i++) {
        if (snapshot.players[i].id == snapshot.activePlayerId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::string GameWindow::winnerText(const GameSnapshot& snapshot) {
    const auto& a = snapshot.players[0];
    const auto& b = snapshot.players[1];
    if (a.territoryCount == b.territoryCount) {
        return "draw";
    }
    int winnerIndex = a.territoryCount > b.territoryCount ? 0 : 1;
    return "Player " + std::to_string(winnerIndex + 1) + " wins";
}

} // namespace territorygame::gui
