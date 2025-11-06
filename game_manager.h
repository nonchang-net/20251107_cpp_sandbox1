#ifndef GAME_MANAGER_H__
#define GAME_MANAGER_H__

#include <SDL3/SDL.h>

#include <iostream>
#include <string>

#include "draw_helper.h"
#include "game/snake.h"
#include "game/test_impl_1.h"
#include "game_constant.h"

namespace MyGame {

class GameManager {
 private:
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  SDL_Joystick* joystick = NULL;
  DrawHelper* painter = NULL;
  TestImpl1::TestImpl1* impl1 = NULL;
  SnakeGame* snakeGame = NULL;

 public:
  ~GameManager() {
    if (joystick) {
      SDL_CloseJoystick(joystick);
    }
    // std::cout << "game manager deconstructed." << std::endl;
  }
  const SDL_AppResult init();
  const SDL_AppResult update();
  const void addJoystick(SDL_Event* event);
  const void removeJoystick(SDL_Event* event);
  const void handleHatEvent(Uint8);
  const void handleKeyEvent(SDL_Scancode);
};

#pragma region HANDLER

const void GameManager::addJoystick(SDL_Event* event) {
  if (joystick == NULL) {
    joystick = SDL_OpenJoystick(event->jdevice.which);
    if (!joystick) {
      SDL_Log("Failed to open joystick ID %u: %s",
              (unsigned int)event->jdevice.which, SDL_GetError());
    }
  }
}
const void GameManager::removeJoystick(SDL_Event* event) {
  if (joystick && (SDL_GetJoystickID(joystick) == event->jdevice.which)) {
    SDL_CloseJoystick(joystick);
    joystick = NULL;
  }
}
const void GameManager::handleHatEvent(Uint8 jhatValue) {
  snakeGame->handleHatEvent(jhatValue);
}
const void GameManager::handleKeyEvent(SDL_Scancode scancode) {
  snakeGame->handleKeyEvent(scancode);
}

#pragma endregion HANDLER

#pragma region INIT

const SDL_AppResult GameManager::init() {
  // std::cout << "hello: ver " << GAME_MANAGER_VERSION << std::endl;

  SDL_SetAppMetadata(APP_TITLE, VERSION_CODE, APP_IDENTIFIER);

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer(APP_TITLE, CANVAS_WIDTH, CANVAS_HEIGHT,
                                   SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderLogicalPresentation(renderer, CANVAS_WIDTH, CANVAS_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  painter = new DrawHelper(renderer);

  // impl1 = new TestImpl1::TestImpl1();
  snakeGame = new SnakeGame(renderer);

  return SDL_APP_CONTINUE;
}

#pragma endregion INIT

#pragma region UPDATE

const SDL_AppResult GameManager::update() {
  // impl1->update(*renderer, *painter);
  // return SDL_APP_CONTINUE;

  return snakeGame->update();
}

#pragma endregion UPDATE

}  // namespace MyGame

#endif