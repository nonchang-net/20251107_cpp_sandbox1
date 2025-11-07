#pragma once

#include <SDL3/SDL.h>

#include <iostream>
#include <string>

// #include "../game/snake.h"
// #include "../game/test_impl_1.h"
#include "../game/test_impl_2.h"
#include "../game_constant.h"
#include "draw_helper.h"
#include "game_impl.h"

namespace MyGame {

class GameManager {
 private:
  // TODO: この辺ってスマートポインタ管理した方が良いやつ？
  SDL_Joystick* joystick = NULL;
  GameImpl* impl;

 public:
  GameManager(GameImpl* impl) : impl(impl) {}
  ~GameManager();
  const void setGameImplementation(GameImpl* impl);
  const SDL_AppResult update();
  const void addJoystick(SDL_Event* event);
  const void removeJoystick(SDL_Event* event);
  const SDL_AppResult handleSdlEvent(SDL_Event* event);
};

}  // namespace MyGame