#include "game_manager.h"

namespace MyGame {

GameManager::~GameManager() {
  if (joystick) {
    SDL_CloseJoystick(joystick);
  }
  // std::cout << "game manager deconstructed." << std::endl;
}

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
// const SDL_AppResult GameManager::handleHatEvent(Uint8 jhatValue) {
//   return snakeGame->handleHatEvent(jhatValue);
// }
// const SDL_AppResult GameManager::handleKeyEvent(SDL_Scancode scancode) {
//   return snakeGame->handleKeyEvent(scancode);
// }
// const SDL_AppResult GameManager::handleUserEvent(SDL_Event* event) {
//   return snakeGame->handleUserEvent(event);
// }
const SDL_AppResult GameManager::handleSdlEvent(SDL_Event* event) {
  return impl->handleSdlEvent(event);
}

#pragma endregion HANDLER

#pragma region INIT

// const void GameManager::setGameImplementation(GameImpl* impl_) { impl =
// impl_; }

// const void GameManager::init() {
//   // std::cout << "hello: ver " << GAME_MANAGER_VERSION << std::endl;

//   // painter = new DrawHelper(renderer);

//   // impl1 = new TestImpl1::TestImpl1();
//   // snakeGame = new SnakeGame::SnakeGame(renderer);
// }

#pragma endregion INIT

#pragma region UPDATE

const SDL_AppResult GameManager::update() {
  // impl1->update(*renderer, *painter);
  // return SDL_APP_CONTINUE;

  // return snakeGame->update();
  return impl->update();
}

#pragma endregion UPDATE
}  // namespace MyGame
