# first: cmake configure: cmake -S . -B build
# build and run: cmake --build build && ./build/main

cmake_minimum_required(VERSION 4.0)

project(MyProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # IntelliSenseのためにcompile_commands.jsonを生成

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# sound library (Phase 2: core classes + sequencer)
set(SOUND_SOURCES
    sound/core/oscillator.cc
    sound/core/envelope.cc
    sound/core/synthesizer.cc
    sound/sequencer/sequencer.cc
    sound/sequencer/multi_track_sequencer.cc
    sound/sequencer/bgm_manager.cc
)
add_library(sound ${SOUND_SOURCES})
target_link_libraries(sound PRIVATE SDL3::SDL3)
target_include_directories(sound PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# game_manager
# note: GameManagerはテンプレートクラスになったため、ヘッダオンリーライブラリです
add_library(game_manager game_manager/draw_helper.cc)
target_link_libraries(game_manager PRIVATE SDL3::SDL3 sound)

# game
# note: file globは追加のたびにconfigureが必要とのことなので保留
# ex: file(GLOB_RECURSE GAME_SOURCES "game/*.cc")
set(GAME_SOURCES
    game/snake.cc
    game/test_impl_2.cc
)
add_library(games ${GAME_SOURCES})
target_link_libraries(games PRIVATE SDL3::SDL3)

# main
add_executable(main game.cc)
target_link_libraries(main PRIVATE SDL3::SDL3 sound game_manager games)