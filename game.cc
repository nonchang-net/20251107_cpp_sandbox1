// note: build & run: cmake --build build && ./build/main

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "game_constant.h"
#include "game_manager/game_manager.h"

typedef struct {
  MyGame::GameManager* gameManager = NULL;
  MyGame::GameImpl* gameImpl = NULL;
} AppState;

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
  MyGame::GameImpl* gameImpl = new MyGame::TestImpl2(renderer);
  MyGame::GameManager* gameManager = new MyGame::GameManager(gameImpl);

  *appstate = as;
  as->gameManager = gameManager;
  as->gameImpl = gameImpl;
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  AppState* as = (AppState*)appstate;
  // return as->gameImpl->handleSdlEvent(event);
  return as->gameManager->handleSdlEvent(event);
  // switch (event->type) {
  //   case SDL_EVENT_QUIT:
  //     return SDL_APP_SUCCESS;
  //   case SDL_EVENT_JOYSTICK_ADDED:
  //     as->gameManager.addJoystick(event);
  //     break;
  //   case SDL_EVENT_JOYSTICK_REMOVED:
  //     as->gameManager.removeJoystick(event);
  //     break;
  //   case SDL_EVENT_JOYSTICK_HAT_MOTION:
  //     return as->gameManager.handleHatEvent(event->jhat.value);
  //   case SDL_EVENT_KEY_DOWN:
  //     return as->gameManager.handleKeyEvent(event->key.scancode);
  //   case SDL_EVENT_USER:
  //     return as->gameManager.handleUserEvent(event);
  //   default:
  //     break;
  // }
  // return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  AppState* as = (AppState*)appstate;
  return as->gameManager->update();
  // return as->gameImpl->update();
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}