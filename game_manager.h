#ifndef GAME_MANAGER_H__
#define GAME_MANAGER_H__

#include <SDL3/SDL.h>

#include <iostream>
#include <string>

#include "game_constant.h"
#include "draw_helper.h"
#include "game/test_impl_1.h"

namespace MyGame {

class GameManager {
 private:
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  DrawHelper* painter = NULL;
  TestImpl1::TestImpl1* impl1 = NULL;

 public:
  const SDL_AppResult init();
  const SDL_AppResult update();
};

#pragma region INIT

const SDL_AppResult GameManager::init() {
  std::cout << "hello: ver " << GAME_MANAGER_VERSION << std::endl;

  SDL_SetAppMetadata("My SDL3 Sandbox1", "0.0.1", "net.nonchang.sdl3.sandbox1");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("examples/renderer/primitives", CANVAS_WIDTH,
                                   CANVAS_HEIGHT, SDL_WINDOW_RESIZABLE, &window,
                                   &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderLogicalPresentation(renderer, CANVAS_WIDTH, CANVAS_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  painter = new DrawHelper(renderer);

  impl1 = new TestImpl1::TestImpl1();

  return SDL_APP_CONTINUE;
}

#pragma endregion INIT

#pragma region UPDATE

const SDL_AppResult GameManager::update() {
  impl1->update(*renderer, *painter);
  return SDL_APP_CONTINUE;
}

#pragma endregion UPDATE

}  // namespace MyGame

#endif