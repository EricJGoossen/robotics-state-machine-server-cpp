cmake_minimum_required(VERSION 3.16)
project(territory_capture_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CPP_ROOT ${CMAKE_SOURCE_DIR}/src/main/cpp)
set(TEST_CPP_ROOT ${CMAKE_SOURCE_DIR}/src/test/cpp)

find_package(Threads REQUIRED)

# Game logic shared by the console binary and the test suite (everything
# under src/main/cpp except Main.cpp itself).
add_library(territory_capture_lib STATIC
    ${CPP_ROOT}/territorygame/domain/Agent.cpp
    ${CPP_ROOT}/territorygame/domain/Player.cpp
    ${CPP_ROOT}/territorygame/domain/Board.cpp
    ${CPP_ROOT}/territorygame/domain/GameState.cpp
    ${CPP_ROOT}/territorygame/domain/GameConfig.cpp
    ${CPP_ROOT}/territorygame/helpers/MovementUtils.cpp
    ${CPP_ROOT}/territorygame/helpers/ObservedBoard.cpp
    ${CPP_ROOT}/territorygame/rules/RespawnService.cpp
    ${CPP_ROOT}/territorygame/rules/TerritoryResolver.cpp
    ${CPP_ROOT}/territorygame/rules/MoveResolver.cpp
    ${CPP_ROOT}/territorygame/visibility/VisibilityService.cpp
    ${CPP_ROOT}/territorygame/engine/GameApiImpl.cpp
    ${CPP_ROOT}/territorygame/engine/TurnManager.cpp
    ${CPP_ROOT}/territorygame/engine/GameEngine.cpp
    ${CPP_ROOT}/territorygame/controller/EnemyStateMachine.cpp
    ${CPP_ROOT}/territorygame/controller/AvailableControllers.cpp
    ${CPP_ROOT}/candidate/examples/BasicStateMachine.cpp
    ${CPP_ROOT}/candidate/examples/RandomStateMachine.cpp
)
target_include_directories(territory_capture_lib PUBLIC ${CPP_ROOT})
target_link_libraries(territory_capture_lib PUBLIC Threads::Threads)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(territory_capture_lib PRIVATE -Wall -Wextra)
endif()

add_executable(territory_capture ${CPP_ROOT}/territorygame/Main.cpp)
target_link_libraries(territory_capture PRIVATE territory_capture_lib)

# GUI binary (SDL2 + Dear ImGui), mirroring Main.java/gui/*. Both are apt
# packages (libsdl2-dev, libimgui-dev); the two backend glue files under
# vendor/imgui_backends/ are vendored source, matching upstream Dear ImGui's
# own convention of shipping backends as files to copy into your project
# rather than a compiled library.
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(SDL2 sdl2)
    pkg_check_modules(IMGUI imgui)
endif()

if(SDL2_FOUND AND IMGUI_FOUND)
    # IMGUI_INCLUDE_DIRS is a list (imgui's own headers plus its stb
    # dependency); take just the imgui root to locate its backends/
    # subdirectory, which Debian's packaging ships headers-only under.
    list(GET IMGUI_INCLUDE_DIRS 0 IMGUI_ROOT_INCLUDE_DIR)

    add_executable(territory_capture_gui
        ${CPP_ROOT}/territorygame/GuiMain.cpp
        ${CPP_ROOT}/territorygame/gui/BoardPanel.cpp
        ${CPP_ROOT}/territorygame/gui/GameWindow.cpp
        ${CPP_ROOT}/vendor/imgui_backends/imgui_impl_sdl.cpp
        ${CPP_ROOT}/vendor/imgui_backends/imgui_impl_sdlrenderer.cpp
    )
    target_include_directories(territory_capture_gui PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${IMGUI_INCLUDE_DIRS}
        ${IMGUI_ROOT_INCLUDE_DIR}/backends
    )
    target_link_libraries(territory_capture_gui PRIVATE
        territory_capture_lib
        ${SDL2_LIBRARIES}
        ${IMGUI_LIBRARIES}
    )
    target_compile_options(territory_capture_gui PRIVATE ${SDL2_CFLAGS_OTHER} ${IMGUI_CFLAGS_OTHER})
    target_link_directories(territory_capture_gui PRIVATE ${SDL2_LIBRARY_DIRS} ${IMGUI_LIBRARY_DIRS})
else()
    message(STATUS "Skipping territory_capture_gui: install libsdl2-dev and libimgui-dev to build it.")
endif()

add_executable(territory_capture_tests
    ${TEST_CPP_ROOT}/RunTests.cpp
    ${TEST_CPP_ROOT}/territorygame/domain/BoardTest.cpp
    ${TEST_CPP_ROOT}/territorygame/domain/GameConfigTest.cpp
    ${TEST_CPP_ROOT}/territorygame/helpers/MovementUtilsTest.cpp
    ${TEST_CPP_ROOT}/territorygame/helpers/ObservedBoardTest.cpp
    ${TEST_CPP_ROOT}/territorygame/rules/MoveResolverTest.cpp
    ${TEST_CPP_ROOT}/territorygame/rules/TerritoryResolverTest.cpp
    ${TEST_CPP_ROOT}/territorygame/rules/RespawnServiceTest.cpp
    ${TEST_CPP_ROOT}/territorygame/visibility/VisibilityServiceTest.cpp
    ${TEST_CPP_ROOT}/territorygame/engine/GameApiImplTest.cpp
    ${TEST_CPP_ROOT}/territorygame/engine/GameSnapshotTest.cpp
    ${TEST_CPP_ROOT}/territorygame/engine/TurnManagerTest.cpp
    ${TEST_CPP_ROOT}/territorygame/engine/GameEngineTest.cpp
    ${TEST_CPP_ROOT}/candidate/examples/BasicStateMachineTest.cpp
)
target_include_directories(territory_capture_tests PRIVATE ${TEST_CPP_ROOT})
target_link_libraries(territory_capture_tests PRIVATE territory_capture_lib)

enable_testing()
add_test(NAME territory_capture_tests COMMAND territory_capture_tests)
