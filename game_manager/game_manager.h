#pragma once

#include <SDL3/SDL.h>

#include <iostream>
#include <memory>
#include <string>

#include "game_impl.h"

namespace MyGame {

class GameManager {
 private:
  // SDL_Joystick* joystick = NULL;
  std::unique_ptr<SDL_Joystick, decltype(&SDL_CloseJoystick)> joystick{
      nullptr, SDL_CloseJoystick};
  std::unique_ptr<GameImpl> impl;

 public:
  // GameManager(GameImpl* impl) : impl(impl) {}
  explicit GameManager(std::unique_ptr<GameImpl> impl)
      : impl(std::move(impl)) {}
  ~GameManager();
  SDL_AppResult update();
  void addJoystick(SDL_Event* event);
  void removeJoystick(SDL_Event* event);
  SDL_AppResult handleSdlEvent(SDL_Event* event);
};

}  // namespace MyGame