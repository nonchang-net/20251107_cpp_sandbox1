# first: cmake configure: cmake -S . -B build
# build and run: cmake --build build && ./build/main

cmake_minimum_required(VERSION 4.0)

project(MyProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_executable(main game.cc)
target_link_libraries(main PRIVATE SDL3::SDL3)