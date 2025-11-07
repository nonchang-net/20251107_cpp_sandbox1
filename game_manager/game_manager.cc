#include "game_manager.h"

namespace MyGame {

GameManager::~GameManager() {
  if (joystick) {
    SDL_CloseJoystick(joystick);
  }
  // std::cout << "game manager deconstructed." << std::endl;
}

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

const SDL_AppResult GameManager::handleSdlEvent(SDL_Event* event) {
  return impl->handleSdlEvent(event);
}

const SDL_AppResult GameManager::update() { return impl->update(); }

#pragma endregion UPDATE
}  // namespace MyGame
