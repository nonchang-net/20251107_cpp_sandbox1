// note: build & run: cmake --build build && ./build/main

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "game_manager.h"

typedef struct {
  MyGame::GameManager gameManager;
} AppState;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
  AppState* as = (AppState*)SDL_calloc(1, sizeof(AppState));
  if (!as) {
    return SDL_APP_FAILURE;
  }
  *appstate = as;
  return as->gameManager.init();
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  AppState* as = (AppState*)appstate;
  switch (event->type) {
    case SDL_EVENT_QUIT:
      return SDL_APP_SUCCESS;
    case SDL_EVENT_JOYSTICK_ADDED:
      as->gameManager.addJoystick(event);
      break;
    case SDL_EVENT_JOYSTICK_REMOVED:
      as->gameManager.removeJoystick(event);
      break;
    case SDL_EVENT_JOYSTICK_HAT_MOTION:
      return as->gameManager.handleHatEvent(event->jhat.value);
    case SDL_EVENT_KEY_DOWN:
      return as->gameManager.handleKeyEvent(event->key.scancode);
    case SDL_EVENT_USER:
      return as->gameManager.handleUserEvent(event);
    default:
      break;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  AppState* as = (AppState*)appstate;
  return as->gameManager.update();
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}