#pragma once

#include <SDL3/SDL.h>

#include <iostream>

#include "../game_manager/draw_helper.h"
#include "../game_manager/game_impl.h"
namespace MyGame {

class TestImpl2 : public GameImpl {
  SDL_Renderer* renderer = NULL;
  DrawHelper painter = NULL;

 public:
  TestImpl2(SDL_Renderer* renderer) : renderer(renderer) {};
  //   SDL_AppResult init(SDL_Renderer*);
  SDL_AppResult handleSdlEvent(SDL_Event*);
  SDL_AppResult update();
};

// ==========================
// TODO: TODO: とりあえずヘッダ内で直
//
// SDL_AppResult TestImpl2::init(SDL_Renderer* renderer_) {
//   // TODO: ベースクラスのinitを隠蔽してるよねこれ？
//   renderer = renderer_;
//   painter = DrawHelper(renderer_);
//   return SDL_APP_CONTINUE;
// }
// TestImpl2::TestImpl2(SDL_Renderer* renderer) : renderer(renderer) {}
SDL_AppResult TestImpl2::handleSdlEvent(SDL_Event* event) {
  std::cout << "TestImpl2::handleSdlEvent() called!" << std::endl;
  return SDL_APP_CONTINUE;
}
SDL_AppResult TestImpl2::update() {
  std::cout << "TestImpl2::update() called!" << std::endl;
  return SDL_APP_CONTINUE;
}
}  // namespace MyGame