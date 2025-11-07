// note: build & run: cmake --build build && ./build/main

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "game_manager.h"

// undone: このstatic持ちやめる方法ないかね?
static MyGame::GameManager gameManager;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
  return gameManager.init();
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_QUIT:
      return SDL_APP_SUCCESS;
    case SDL_EVENT_JOYSTICK_ADDED:
      gameManager.addJoystick(event);
      break;
    case SDL_EVENT_JOYSTICK_REMOVED:
      gameManager.removeJoystick(event);
      break;
    case SDL_EVENT_JOYSTICK_HAT_MOTION:
      return gameManager.handleHatEvent(event->jhat.value);
    case SDL_EVENT_KEY_DOWN:
      return gameManager.handleKeyEvent(event->key.scancode);
    case SDL_EVENT_USER:
      return gameManager.handleUserEvent(event);
    default:
      break;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) { return gameManager.update(); }

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}