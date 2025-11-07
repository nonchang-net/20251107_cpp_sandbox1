# first: cmake configure: cmake -S . -B build
# build and run: cmake --build build && ./build/main

cmake_minimum_required(VERSION 4.0)

project(MyProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# game_manager
add_library(game_manager game_manager/game_manager.cc game_manager/draw_helper.cc)
target_link_libraries(game_manager PRIVATE SDL3::SDL3)

# game
add_library(games game/snake.cc game/test_impl_2.cc)
target_link_libraries(games PRIVATE SDL3::SDL3)

# main
add_executable(main game.cc)
target_link_libraries(main PRIVATE SDL3::SDL3 game_manager games)