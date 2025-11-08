// note: build & run: cmake --build build && ./build/main

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <memory>

#include "game/snake.h"
#include "game/test_impl_2.h"
#include "game_constant.h"
#include "game_manager/game_manager.h"

// 使用するゲーム実装の型を選択
using CurrentGameType = MyGame::TestImpl2;
// using CurrentGameType = MyGame::SnakeGame::SnakeGame;

struct AppState {
  std::unique_ptr<MyGame::GameManager<CurrentGameType>> gameManager;
};

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
  AppState* as = (AppState*)SDL_calloc(1, sizeof(AppState));
  if (!as) {
    return SDL_APP_FAILURE;
  }

  SDL_SetAppMetadata(MyGame::APP_TITLE, MyGame::VERSION_CODE,
                     MyGame::APP_IDENTIFIER);

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  if (!SDL_CreateWindowAndRenderer(MyGame::APP_TITLE, MyGame::CANVAS_WIDTH,
                                   MyGame::CANVAS_HEIGHT, SDL_WINDOW_RESIZABLE,
                                   &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderLogicalPresentation(renderer, MyGame::CANVAS_WIDTH,
                                   MyGame::CANVAS_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  // ゲーム実装初期化
  auto gameImpl = std::make_unique<CurrentGameType>(renderer);

  // placement newでAppStateをSDL_callocで確保済みの領域に構築
  as = new (as) AppState{
      std::make_unique<MyGame::GameManager<CurrentGameType>>(std::move(gameImpl))};

  *appstate = as;
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_QUIT:
      return SDL_APP_SUCCESS;
    default:
      break;
  }
  // return SDL_APP_CONTINUE;

  AppState* as = (AppState*)appstate;
  // note: implに直でhandleさせるのは違うかな、を検討中
  // return as->gameImpl->handleSdlEvent(event);
  return as->gameManager->handleSdlEvent(event);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  AppState* as = (AppState*)appstate;
  // note: implに直でhandleさせるのは違うかな、を検討中
  // return as->gameImpl->update();
  return as->gameManager->update();
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  if (appstate != nullptr) {
    AppState* as = (AppState*)appstate;
    // note: placement newで構築したので明示的にデストラクタを呼ぶ
    as->~AppState();
    SDL_free(as);
  }
}