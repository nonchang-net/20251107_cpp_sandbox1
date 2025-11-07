#pragma once

#include <SDL3/SDL.h>

#include "../draw_helper.h"
#include "../game_constant.h"

namespace MyGame::TestImpl1 {

class TestImpl1 {
 private:
  SDL_FPoint points[500];

 public:
  const void update(SDL_Renderer& renderer, DrawHelper& painter);
};

}  // namespace MyGame::TestImpl1
