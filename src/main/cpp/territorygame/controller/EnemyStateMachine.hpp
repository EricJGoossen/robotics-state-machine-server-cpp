#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "territorygame/api/AgentController.hpp"
#include "territorygame/api/Direction.hpp"
#include "territorygame/api/GameApi.hpp"
#include "territorygame/api/GridPosition.hpp"
#include "territorygame/api/OccupantView.hpp"
#include "territorygame/api/TerritoryView.hpp"
#include "territorygame/api/VisibleCell.hpp"

namespace territorygame::controller {

// Framework code: the standard opponent used for assessment runs. Not part
// of the candidate-facing surface.
//
// Five states, organized by risk posture and picked fresh every turn
// (highest priority first):
//   DEFENSIVE  - our owned territory just shrank since last turn, meaning an
//                opponent capture is in progress or just landed. Chase their
//                visible trail for a kill if we can see one; otherwise fall
//                back to heading home.
//   RECEDING   - safe. Our trail is long, we're low enough on turns that
//                pushing further risks not making it back, the opponent is
//                visible and close while we're exposed, or we're already
//                ahead and the match is nearly over. Head for the nearest
//                owned territory, or shuffle around inside it if we're
//                already there.
//   AGGRESSIVE - risky. The opponent's trail or territory is visible and
//                we're not currently in danger; go take it.
//   EXPANDING  - the default. Deterministically push toward whichever safe
//                direction opens onto the most free space.
//   WANDERING  - a rare, single-turn detour: pick at random among directions
//                whose open-space score is merely close to the best.
//
// Earlier versions of this bot got stuck in short back-and-forth loops. The
// deeper problem is that two instances of the same deterministic logic
// playing each other can settle into a stable cycle that isn't a repeat of
// either agent's own positions at all. Rather than detect specific cycle
// shapes, WANDERING periodically (and briefly) makes the whole match
// non-deterministic, which is enough to knock either agent off any cycle
// regardless of its length.
class EnemyStateMachine final : public territorygame::api::AgentController {
public:
  EnemyStateMachine();
  // Two instances of this same deterministic logic need different seeds, or
  // they'll play out identically for long stretches.
  explicit EnemyStateMachine(int64_t seed);

  void takeTurn(territorygame::api::GameApi &game) override;
  std::optional<std::string> getDebugState() const override;

private:
  enum class State { DEFENSIVE, RECEDING, AGGRESSIVE, EXPANDING, WANDERING };

  static constexpr int MAX_TRAIL_BEFORE_RETURN = 8;
  static constexpr int SAFETY_TURN_BUFFER = 4;
  static constexpr int CONSOLIDATE_TURNS_THRESHOLD = 30;
  static constexpr int WANDER_OPENNESS_TOLERANCE = 1;
  static constexpr double RANDOM_WANDER_CHANCE = 0.01;

  // ---- State selection ----
  State decideState(territorygame::api::GameApi &game, State previousState);
  bool shouldRecede(territorygame::api::GameApi &game);
  bool shouldBeAggressive(territorygame::api::GameApi &game);
  bool opponentIsThreateninglyClose(territorygame::api::GameApi &game);
  bool isEndgameWithLead(territorygame::api::GameApi &game);

  // ---- Direction selection ----
  territorygame::api::Direction
  chooseDirection(territorygame::api::GameApi &game, State state);
  territorygame::api::Direction
  pickDefensive(territorygame::api::GameApi &game);
  territorygame::api::Direction
  pickExpanding(territorygame::api::GameApi &game);
  territorygame::api::Direction pickReceding(territorygame::api::GameApi &game);
  territorygame::api::Direction
  pickAggressive(territorygame::api::GameApi &game);
  std::optional<territorygame::api::Direction>
  huntOpponentTrail(territorygame::api::GameApi &game);
  territorygame::api::Direction
  pickWandering(territorygame::api::GameApi &game);
  territorygame::api::Direction
  pickUniformlyRandom(territorygame::api::GameApi &game);

  // Picks the candidate with the smallest key(direction); ties broken by
  // first occurrence, matching Stream.min()'s stable behavior over an
  // ordered source. Falls back to fallback() if candidates is empty.
  territorygame::api::Direction
  chooseBest(const std::vector<territorygame::api::Direction> &candidates,
             const std::function<int(territorygame::api::Direction)> &key);

  // ---- Board reading ----
  std::vector<territorygame::api::Direction>
  safeDirections(territorygame::api::GameApi &game);
  std::vector<territorygame::api::Direction>
  huntableDirections(territorygame::api::GameApi &game);
  int openNeighborCount(territorygame::api::GameApi &game,
                        territorygame::api::Direction direction);
  std::optional<territorygame::api::GridPosition>
  nearestOccupant(territorygame::api::GameApi &game,
                   territorygame::api::OccupantView occupant);
  std::optional<territorygame::api::GridPosition>
  nearestTerritory(territorygame::api::GameApi &game,
                    territorygame::api::TerritoryView territory);
  std::optional<territorygame::api::GridPosition>
  nearestVisible(territorygame::api::GameApi &game,
                 const std::function<bool(const territorygame::api::VisibleCell &)> &match);
  territorygame::api::GridPosition
  destination(territorygame::api::GameApi &game,
              territorygame::api::Direction direction);
  // Absent when position is off the board (a corner or edge), not merely an open cell.
  std::optional<territorygame::api::VisibleCell>
  cellAt(territorygame::api::GameApi &game,
         territorygame::api::GridPosition position);
  std::optional<territorygame::api::OccupantView>
  occupantAt(territorygame::api::GameApi &game,
             territorygame::api::GridPosition position);
  std::optional<territorygame::api::TerritoryView>
  territoryAt(territorygame::api::GameApi &game,
              territorygame::api::GridPosition position);
  static bool isOpen(const std::optional<territorygame::api::VisibleCell> &cell);
  territorygame::api::Direction fallback();

  std::mt19937_64 random_;
  State currentState_ = State::EXPANDING;
  int previousOwnedTerritoryCount_ = 0;
  bool firstMove_ = true;
  std::optional<territorygame::api::Direction> direction_;
  int bestOpenNeighborCount_ = 0;
};

} // namespace territorygame::controller
