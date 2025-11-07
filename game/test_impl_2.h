#pragma once

#include <SDL3/SDL.h>

#include <iostream>

#include "../game_manager/draw_helper.h"
#include "../game_manager/game_impl.h"
namespace MyGame {

class TestImpl2 : public GameImpl {
 private:
  SDL_Renderer* renderer = NULL;
  DrawHelper painter = NULL;
  // test impl
  SDL_FPoint points[500];

 public:
  TestImpl2(SDL_Renderer* renderer) : renderer(renderer), painter(renderer) {};
  SDL_AppResult handleSdlEvent(SDL_Event*);
  SDL_AppResult update();
};

}  // namespace MyGame