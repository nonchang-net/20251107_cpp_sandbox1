#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <iostream>

#include "../game_manager/draw_helper.h"
#include "../game_manager/game_impl.h"
namespace MyGame {

class TestImpl2 : public GameImpl {
 private:
  SDL_Renderer* renderer = nullptr;
  DrawHelper painter;
  std::array<SDL_FPoint, 500> points;

 public:
  TestImpl2(SDL_Renderer* renderer) : renderer(renderer), painter(renderer) {};
  SDL_AppResult handleSdlEvent(SDL_Event*);
  SDL_AppResult update();
};

}  // namespace MyGame