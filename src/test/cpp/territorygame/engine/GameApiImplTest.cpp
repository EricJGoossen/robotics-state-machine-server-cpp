#include "testing/TestRunner.hpp"

#include "territorygame/TestGames.hpp"
#include "territorygame/engine/GameApiImpl.hpp"
#include "territorygame/rules/MoveResolver.hpp"
#include "territorygame/rules/RespawnService.hpp"
#include "territorygame/rules/TerritoryResolver.hpp"
#include "territorygame/visibility/VisibilityService.hpp"

using namespace territorygame::api;
using territorygame::domain::GameState;
using territorygame::domain::PlayerId;
using territorygame::engine::GameApiImpl;
using territorygame::rules::MoveResolver;
using territorygame::rules::RespawnService;
using territorygame::rules::TerritoryResolver;
using territorygame::visibility::VisibilityService;

namespace {
const PlayerId player0{0};
} // namespace

TEST(GameApiImplTest, firstMoveThisTurnResolvesNormally) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, {GridPosition{1, 1}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver moveResolver{RespawnService(), TerritoryResolver()};
    VisibilityService visibilityService(5);
    GameApiImpl api(state, player0, moveResolver, visibilityService);
    api.resetForNewTurn();

    MoveResult result = api.move(Direction::EAST);

    EXPECT_EQ(result, MoveResult::MOVED);
}

TEST(GameApiImplTest, secondSuccessfulMoveInTheSameTurnIsRejected) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, {GridPosition{1, 1}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver moveResolver{RespawnService(), TerritoryResolver()};
    VisibilityService visibilityService(5);
    GameApiImpl api(state, player0, moveResolver, visibilityService);
    api.resetForNewTurn();

    api.move(Direction::EAST);
    MoveResult secondResult = api.move(Direction::EAST);

    EXPECT_EQ(secondResult, MoveResult::INVALID);
    // Position reflects only the first move, not two.
    EXPECT_EQ(state.getPlayer(player0).getAgent().getPosition(), (GridPosition{2, 1}));
}

TEST(GameApiImplTest, resetForNewTurnAllowsAnotherSuccessfulMove) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{1, 1}, {GridPosition{1, 1}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver moveResolver{RespawnService(), TerritoryResolver()};
    VisibilityService visibilityService(5);
    GameApiImpl api(state, player0, moveResolver, visibilityService);
    api.resetForNewTurn();
    api.move(Direction::EAST);

    api.resetForNewTurn();
    MoveResult result = api.move(Direction::EAST);

    EXPECT_EQ(result, MoveResult::MOVED);
    EXPECT_EQ(state.getPlayer(player0).getAgent().getPosition(), (GridPosition{3, 1}));
}

TEST(GameApiImplTest, invalidMoveDoesNotConsumeTheOneMovePerTurnAllowance) {
    GameState state = territorygame::test::twoPlayerState(
        8, 8, GridPosition{0, 0}, {GridPosition{0, 0}}, GridPosition{6, 6}, {GridPosition{6, 6}}, 10);
    MoveResolver moveResolver{RespawnService(), TerritoryResolver()};
    VisibilityService visibilityService(5);
    GameApiImpl api(state, player0, moveResolver, visibilityService);
    api.resetForNewTurn();

    MoveResult firstResult = api.move(Direction::NORTH); // out of bounds
    MoveResult secondResult = api.move(Direction::EAST); // now a real move

    EXPECT_EQ(firstResult, MoveResult::INVALID);
    EXPECT_EQ(secondResult, MoveResult::MOVED);
}
