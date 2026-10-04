#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "testing/TestRunner.hpp"

#include "territorygame/TestGames.hpp"
#include "territorygame/api/AgentController.hpp"
#include "territorygame/engine/GameApiImpl.hpp"
#include "territorygame/engine/TurnManager.hpp"
#include "territorygame/rules/MoveResolver.hpp"
#include "territorygame/rules/RespawnService.hpp"
#include "territorygame/rules/TerritoryResolver.hpp"
#include "territorygame/visibility/VisibilityService.hpp"

using namespace territorygame::api;
using territorygame::domain::GameState;
using territorygame::domain::PlayerId;
using territorygame::engine::GameApiImpl;
using territorygame::engine::TurnManager;
using territorygame::rules::MoveResolver;
using territorygame::rules::RespawnService;
using territorygame::rules::TerritoryResolver;
using territorygame::visibility::VisibilityService;

namespace {
const PlayerId player0{0};
const PlayerId player1{1};

// Always moves the same direction.
class AlwaysMoveController final : public AgentController {
public:
    explicit AlwaysMoveController(Direction direction) : direction_(direction) {}
    void takeTurn(GameApi& game) override { game.move(direction_); }

private:
    Direction direction_;
};

// Tries an invalid move once, then always moves EAST after that.
class RetryOnceThenEastController final : public AgentController {
public:
    void takeTurn(GameApi& game) override {
        invocationCount_++;
        if (!triedInvalidMove_) {
            triedInvalidMove_ = true;
            game.move(Direction::NORTH); // agent starts at y=0, guaranteed invalid
        } else {
            game.move(Direction::EAST);
        }
    }

    int getInvocationCount() const { return invocationCount_; }

private:
    bool triedInvalidMove_ = false;
    int invocationCount_ = 0;
};

// Always attempts an out-of-bounds move; never succeeds.
class AlwaysInvalidController final : public AgentController {
public:
    void takeTurn(GameApi& game) override {
        invocationCount_++;
        game.move(Direction::NORTH); // player0 starts at y=0 in these tests, guaranteed invalid
    }

    int getInvocationCount() const { return invocationCount_; }

private:
    int invocationCount_ = 0;
};

// Always throws instead of moving.
class ThrowingController final : public AgentController {
public:
    void takeTurn(GameApi&) override { throw std::runtime_error("boom"); }
};

std::unordered_map<PlayerId, std::unique_ptr<GameApiImpl>> buildApis(
    GameState& state, MoveResolver& moveResolver, VisibilityService& visibilityService) {
    std::unordered_map<PlayerId, std::unique_ptr<GameApiImpl>> apis;
    apis[player0] = std::make_unique<GameApiImpl>(state, player0, moveResolver, visibilityService);
    apis[player1] = std::make_unique<GameApiImpl>(state, player1, moveResolver, visibilityService);
    return apis;
}

std::unordered_map<PlayerId, GameApiImpl*> toRaw(
    const std::unordered_map<PlayerId, std::unique_ptr<GameApiImpl>>& apis) {
    std::unordered_map<PlayerId, GameApiImpl*> raw;
    for (const auto& [id, api] : apis) {
        raw[id] = api.get();
    }
    return raw;
}

} // namespace

TEST(TurnManagerTest, activePlayerAlternatesAfterEachTurn) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, {GridPosition{1, 1}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver moveResolver{RespawnService(), TerritoryResolver()};
    VisibilityService visibilityService(5);
    auto apis = buildApis(state, moveResolver, visibilityService);
    auto apisRaw = toRaw(apis);

    AlwaysMoveController east(Direction::EAST);
    AlwaysMoveController west(Direction::WEST);
    std::unordered_map<PlayerId, AgentController*> controllers{{player0, &east}, {player1, &west}};
    TurnManager turnManager(20);

    EXPECT_EQ(state.getActivePlayerId(), player0);

    turnManager.executeTurn(state, controllers, apisRaw);
    EXPECT_EQ(state.getActivePlayerId(), player1);

    turnManager.executeTurn(state, controllers, apisRaw);
    EXPECT_EQ(state.getActivePlayerId(), player0);
}

TEST(TurnManagerTest, controllerIsInvokedAgainAfterAnInvalidMoveWithinTheSameTurn) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{0, 0}, {GridPosition{0, 0}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver moveResolver{RespawnService(), TerritoryResolver()};
    VisibilityService visibilityService(5);
    auto apis = buildApis(state, moveResolver, visibilityService);
    auto apisRaw = toRaw(apis);

    RetryOnceThenEastController player0Controller;
    AlwaysMoveController west(Direction::WEST);
    std::unordered_map<PlayerId, AgentController*> controllers{{player0, &player0Controller}, {player1, &west}};
    TurnManager turnManager(20);

    turnManager.executeTurn(state, controllers, apisRaw);

    EXPECT_TRUE(player0Controller.getInvocationCount() >= 2);
    // Only the one successful move (turn 2) decremented the turn count.
    EXPECT_EQ(state.getRemainingTurns(player0), 9);
}

TEST(TurnManagerTest, onlyTheActivePlayersRemainingTurnsAreConsumed) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, {GridPosition{1, 1}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver moveResolver{RespawnService(), TerritoryResolver()};
    VisibilityService visibilityService(5);
    auto apis = buildApis(state, moveResolver, visibilityService);
    auto apisRaw = toRaw(apis);

    AlwaysMoveController east(Direction::EAST);
    AlwaysMoveController west(Direction::WEST);
    std::unordered_map<PlayerId, AgentController*> controllers{{player0, &east}, {player1, &west}};
    TurnManager turnManager(20);

    turnManager.executeTurn(state, controllers, apisRaw);

    EXPECT_EQ(state.getRemainingTurns(player0), 9);
    EXPECT_EQ(state.getRemainingTurns(player1), 10);
}

TEST(TurnManagerTest, turnIsForfeitedAfterMaxAttemptsWhenControllerNeverSucceeds) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{0, 0}, {GridPosition{0, 0}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver moveResolver{RespawnService(), TerritoryResolver()};
    VisibilityService visibilityService(5);
    auto apis = buildApis(state, moveResolver, visibilityService);
    auto apisRaw = toRaw(apis);

    TurnManager cappedTurnManager(5);
    AlwaysInvalidController player0Controller;
    AlwaysMoveController west(Direction::WEST);
    std::unordered_map<PlayerId, AgentController*> controllers{{player0, &player0Controller}, {player1, &west}};

    cappedTurnManager.executeTurn(state, controllers, apisRaw);

    EXPECT_EQ(player0Controller.getInvocationCount(), 5);
    EXPECT_EQ(state.getRemainingTurns(player0), 9); // forfeited turn is still consumed
    EXPECT_EQ(state.getActivePlayerId(), player1);
    EXPECT_TRUE(state.getLastTurnError().has_value());
}

TEST(TurnManagerTest, turnIsForfeitedWhenControllerThrows) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, {GridPosition{1, 1}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver moveResolver{RespawnService(), TerritoryResolver()};
    VisibilityService visibilityService(5);
    auto apis = buildApis(state, moveResolver, visibilityService);
    auto apisRaw = toRaw(apis);

    ThrowingController thrower;
    AlwaysMoveController west(Direction::WEST);
    std::unordered_map<PlayerId, AgentController*> controllers{{player0, &thrower}, {player1, &west}};
    TurnManager turnManager(20);

    turnManager.executeTurn(state, controllers, apisRaw);

    EXPECT_EQ(state.getRemainingTurns(player0), 9);
    EXPECT_EQ(state.getActivePlayerId(), player1);
    EXPECT_TRUE(state.getLastTurnError().has_value());
}
