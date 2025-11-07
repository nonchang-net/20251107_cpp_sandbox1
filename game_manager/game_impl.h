#pragma once

#include <SDL3/SDL.h>
namespace MyGame {

class GameImpl {
 public:
  // virtual SDL_AppResult init(SDL_Renderer* my_renderer) {
  //   renderer = my_renderer;
  // }
  virtual SDL_AppResult handleSdlEvent(SDL_Event* event) = 0;
  virtual SDL_AppResult update() = 0;
};

}  // namespace MyGame