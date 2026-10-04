# Candidate Guide

Write your logic in `src/main/cpp/candidate/CandidateController.hpp`.
That's the exact class the framework calls.

You don't need to touch anything outside `src/main/cpp/candidate/`. If you
want more classes, add them in there too, that folder is yours to extend
however you like. Header-only additions need no build changes. The one
exception to "nothing outside it": if you add a `.cpp` file, list it in the
`territory_capture_lib` sources in `CMakeLists.txt` so it gets compiled.

Build and run from the repository root (see `README.md` for setup):

```
cmake -S . -B build && cmake --build build -j
./build/territory_capture_gui     # watch a match
./build/territory_capture         # headless: Enemy State Machine vs. you
```

Inside `candidate/`, `examples/` has a couple of read-only implementations
worth studying. There's also a more advanced opponent to practice against,
shown as "Enemy State Machine" in the GUI.

## The rules

- You move one step at a time: NORTH, SOUTH, EAST, or WEST.
- Step outside your own territory → you start (or extend) a trail.
- Step back onto territory you own → your trail closes and becomes territory.
  Anything it surrounds becomes yours too, including enemy territory.
- Step on your own trail → you die.
- Step on the enemy's trail → they die, you're fine.
- Dying sends you back to your starting zone. You keep playing after that.
- The match ends when both players run out of turns. Most territory wins.
- A move can be `INVALID` (off the board, or into the opponent's current
  spot). Nothing happens, and you get asked to move again.

## `AgentController`

This is the abstract base class `CandidateController` derives from
(`territorygame::api::AgentController`).

### takeTurn(game)
```cpp
void takeTurn(GameApi& game) override
```
Put your decision-making here. You never call this method yourself. The
framework calls it for you, whenever it's your turn.

`game` (a `GameApi`, see below) is your window into the current turn: where
you are, what you can see, how the match stands. Read what you need from
it, decide on a direction, then call `game.move(direction)` **exactly
once**. That call is your whole turn. `takeTurn` doesn't return anything;
`move()` is how you actually act.

If the direction you pick comes back `INVALID`, nothing happens and
`takeTurn` is called again right away so you can try something else, same
turn.

### getDebugState()
```cpp
std::optional<std::string> getDebugState() const override  // default: std::nullopt
```
Optional. Return a short label for whatever state you think
you're in (an enum's name works well), and the GUI shows it next to your
player card. Handy for watching a match and seeing what your controller is
"thinking." No effect on gameplay either way.

## `GameApi`

Your one-turn snapshot of the match. Read what you need, then call
`game.move(direction)` once.

### getAgentPosition()
```cpp
GridPosition getAgentPosition() const
```
Where you are right now.

### getRespawnPosition()
```cpp
GridPosition getRespawnPosition() const
```
Where you reappear after dying.

### getOwnedTerritoryCellCount()
```cpp
int getOwnedTerritoryCellCount() const
```
How many cells you currently own.

### getOpponentTerritoryCellCount()
```cpp
int getOpponentTerritoryCellCount() const
```
How many cells the opponent currently owns.

### getRemainingTurns()
```cpp
int getRemainingTurns() const
```
How many turns you have left.

### getActiveTrail()
```cpp
std::vector<GridPosition> getActiveTrail() const
```
Your current trail, oldest cell first. Empty if you're standing on your own
territory right now.

### getVisibleGrid()
```cpp
std::vector<std::vector<VisibleCell>> getVisibleGrid() const
```
The cells you can currently see, centered on you. Index it `[y][x]`. It's
returned by value, so grab it once per turn rather than calling it in a
loop. Each
`VisibleCell` carries its real board position too, so you never have to
convert window coordinates to board coordinates yourself.

### getBoardWidth() / getBoardHeight()
```cpp
int getBoardWidth() const
int getBoardHeight() const
```
The size of the whole board (not just what you can see).

### move(direction)
```cpp
MoveResult move(Direction direction)
```
Try to move one step. Returns what happened.

## Types

All of these live in namespace `territorygame::api`.

### GridPosition
```cpp
struct GridPosition { int x; int y; };
```
A cell on the board. `(0, 0)` is the top-left corner; x increases right, y
increases down. Supports `==`/`!=` and has a `std::hash` specialization, so
it works as a key in `std::unordered_map`/`std::unordered_set`.

### VisibleCell
```cpp
struct VisibleCell { GridPosition position; OccupantView occupant; TerritoryView territory; };
```
One cell you can see. `occupant` is the head/trail layer; `territory` is
the land underneath. Both are always present -- a trail does not hide
territory.

### Direction
```cpp
enum class Direction { NORTH, SOUTH, EAST, WEST };
inline constexpr std::array<Direction, 4> DIRECTIONS{...};  // all four, for looping
```

### MoveResult
```cpp
enum class MoveResult { MOVED, CAPTURED, DIED, INVALID };
```
What `move()` just did: `MOVED` (normal step), `CAPTURED` (your trail
closed), `DIED` (you hit a trail), or `INVALID` (nothing happened).

### OccupantView
```cpp
enum class OccupantView {
    EMPTY, SELF_TRAIL, OPPONENT_TRAIL, SELF_AGENT, OPPONENT_AGENT
};
```
What's standing or trailing on the cell. You'll never see the opponent's
real player number, only `SELF_*` or `OPPONENT_*`. An agent standing on a
trail shows as the agent, not the trail.

### TerritoryView
```cpp
enum class TerritoryView { UNOWNED, SELF, OPPONENT };
```
Who owns the land. Independent of `occupant`. Empty unowned space is
`(EMPTY, UNOWNED)`.

## Helpers (`territorygame::helpers`)

Basic board math you shouldn't have to write yourself. Free to use from
`CandidateController`. `MovementUtils` functions are all `static`; include
`territorygame/helpers/MovementUtils.hpp` and
`territorygame/helpers/ObservedBoard.hpp`.

### MovementUtils.nextPosition
```cpp
GridPosition nextPosition(GridPosition position, Direction direction)
```
The cell one step away in a direction.

### MovementUtils.isWithinBoard
```cpp
bool isWithinBoard(GridPosition position, int width, int height)
```
Is this cell actually on the board?

### MovementUtils.isValidMove
```cpp
bool isValidMove(const GameApi& game, Direction direction)
```
Would this move be on the board and not walk into the opponent's agent?
**This does not check trails.** Walking into your own trail (or theirs)
still counts as "valid" here. That part is on you.

### MovementUtils.validDirections
```cpp
std::vector<Direction> validDirections(const GameApi& game)
```
All directions that pass `isValidMove` right now.

### MovementUtils.manhattanDistance
```cpp
int manhattanDistance(GridPosition a, GridPosition b)
```
Grid distance between two cells (no diagonals).

### MovementUtils.findCell
```cpp
std::optional<VisibleCell> findCell(
    const std::vector<std::vector<VisibleCell>>& visibleGrid, GridPosition position)
```
Look up one cell in a visible grid by its board position.

### MovementUtils.randomDirection
```cpp
Direction randomDirection(std::mt19937_64& random)
```
Picks one of the four directions at random. Keep the generator as a member
of your controller so it isn't reseeded every turn.

### ObservedBoard
```cpp
ObservedBoard(int width, int height)
void update(const std::vector<std::vector<VisibleCell>>& visibleGrid)   // call this each turn
std::optional<VisibleCell> get(GridPosition position) const
bool hasObserved(GridPosition position) const
void clear()
```
Remembers the last thing you saw at each cell, so you can reason about
parts of the board outside your current window. It only remembers what
you've actually seen. It won't guess whether a cell has changed since.

## Tips

Nothing below is solved for you in the example controllers. Worth thinking
about once the basics are working:

- **Avoid your own trail.** `isValidMove` only checks board bounds and the
  opponent's agent; stepping on your own trail still passes it, but it
  kills you.
- **Watch for the opponent's trail too.** Crossing it kills *them*, and
  their agent's position hints at where their trail might be.
- **Aggression vs. caution.** Compare `getOwnedTerritoryCellCount()` to
  `getOpponentTerritoryCellCount()`. Ahead, maybe hunt for a kill; behind,
  maybe play safer.
- **Getting home efficiently.** `getRespawnPosition()` isn't necessarily
  your nearest owned cell once you've captured territory elsewhere.
  Scanning `getVisibleGrid()` for the nearest `TerritoryView::SELF` cell can do
  better.
- **Trail length is a trade-off.** Longer trails claim more area on
  capture but leave you exposed for longer.
- **Remember what you've seen.** `ObservedBoard` builds a picture beyond
  your current visible window, useful for planning ahead.
- **Remaining turns matter.** Early vs. late game might call for
  different behavior. `getRemainingTurns()` tells you where you stand.
