# Territory Capture

A local two-player territory-capture game, built as a C++ framework for an
interview assessment: the framework owns the board, rules, turn execution,
and GUI; a candidate implements one class — an `AgentController` — to play.

Two agents move around a grid, laying trails outside their own territory
and closing them to capture the enclosed area (and any opponent territory
inside it). Crossing your own trail kills you; crossing an opponent's trail
kills them. Whoever holds more territory when both players run out of
turns wins.

![Territory Capture gameplay](src/main/resources/GamePlayScreenShot.png)

## Requirements

- A C++17 compiler (clang or gcc)
- CMake 3.16+
- For the GUI: SDL2 and Dear ImGui development packages, found through
  `pkg-config`. Without them, CMake skips the GUI and still builds the
  headless binary and the tests.

The included devcontainer (`.devcontainer/`) has all of the above
preinstalled.

## Getting started

### 1. Install dependencies

**Linux / WSL (Debian/Ubuntu):**

```
sudo apt install build-essential cmake pkg-config libsdl2-dev libimgui-dev
```

Or open the repository in VS Code and choose **Reopen in Container** to use
the devcontainer instead.

### 2. Clone the repository

```
git clone https://github.com/EricJGoossen/robotics-state-machine-server-cpp.git
cd robotics-state-machine-server-cpp
```

### 3. Build

```
cmake -S . -B build
cmake --build build -j
```

### 4. Run

Run everything from the repository root, because the game loads
`src/main/resources/game-config.properties` relative to the current
directory.

```
./build/territory_capture_gui     # GUI
./build/territory_capture         # headless: Enemy State Machine vs. Candidate Controller
./build/territory_capture_tests   # test suite
```

In the GUI, pick which controller occupies each player slot (Basic State
Machine, Enemy State Machine, Random State Machine, or the candidate's own
controller), then use Start / Pause / Step / Reset to run a match.

The headless binary plays one match at full speed and prints a summary
every 200 turns plus the final result. It's handy for quick iteration or
scripting.

Inside the devcontainer, `build` and `run` (build, then launch the GUI) are
available as shell aliases.

## Candidate assessment

See `CANDIDATE_GUIDE.md` for the assessment task: the file to edit, the
rules from a player's perspective, the API reference, and some tips.

## Project structure

```
CMakeLists.txt               build: territory_capture_lib (all game logic),
                              territory_capture, territory_capture_gui,
                              territory_capture_tests

src/main/resources/
  game-config.properties     board size, visibility, turn count, respawn
                              positions, starting-territory size

src/main/cpp/territorygame/
  api/          Candidate-facing types: GameApi, AgentController,
                GridPosition, VisibleCell, OccupantView, TerritoryView,
                Direction, MoveResult. Nothing outside this namespace is
                ever handed to candidate code.

  domain/       Authoritative game state: PlayerId, GameConfig, Agent,
                Player, Board, GameState. Not exposed to candidates or
                the GUI.

  rules/        The actual rules, isolated from turn management:
                MoveResolver (one move's resolution order), TerritoryResolver
                (flood-fill capture), RespawnService (death/reset).

  visibility/   VisibilityService — builds a candidate's visible-cell
                window and translates internal player identities to
                SELF_*/OPPONENT_* types.

  engine/       Match orchestration: GameEngine (lifecycle, Start/Pause/
                Step/Reset, runs on a background thread), TurnManager (one
                turn, controller retry-on-invalid), GameApiImpl (per-player
                facade over GameApi), GameSnapshot/GameObserver (the
                read-only view the GUI renders from).

  helpers/      Provided utilities candidates may use: ObservedBoard
                (remembers the latest observed value per cell),
                MovementUtils (position/bounds arithmetic, mechanical move
                validation, Manhattan distance).

  controller/   Framework-internal AgentController implementations not
                meant as examples to copy: EnemyStateMachine (the
                standard assessment opponent) and AvailableControllers
                (the static registry the GUI's controller picker reads
                from — Basic State Machine, Enemy State Machine, Random
                State Machine, Candidate Controller).

  gui/          SDL2 + Dear ImGui viewer: GameWindow (controls, status,
                wiring) and BoardPanel (board rendering). No game-rule
                logic lives here.

  GuiMain.cpp   GUI entry point — creates the window and runs the render
                loop.

  Main.cpp      Headless entry point — runs one match and prints results.

src/main/cpp/candidate/
  CandidateController.hpp    the file to edit for the assessment.

  examples/                  Read-only reference controllers: BasicStateMachine
                              (a deliberately weak state-machine example) and
                              RandomStateMachine (the simplest possible baseline).
                              Not templates for a good strategy.

src/main/cpp/vendor/         Dear ImGui SDL2 backend sources (vendored).

src/test/cpp/...             mirrors the main layout; RunTests.cpp plus a
                              small built-in test runner (testing/).
```
