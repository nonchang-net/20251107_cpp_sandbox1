#include "game_manager.h"

namespace MyGame {

GameManager::~GameManager() {
  // unique_ptrは自動的にデストラクタで解放されるため、明示的な処理は不要
  // std::cout << "game manager deconstructed." << std::endl;
}

void GameManager::addJoystick(SDL_Event* event) {
  if (!joystick) {
    joystick.reset(SDL_OpenJoystick(event->jdevice.which));
    if (!joystick) {
      SDL_Log("Failed to open joystick ID %u: %s",
              (unsigned int)event->jdevice.which, SDL_GetError());
    }
  }
}
void GameManager::removeJoystick(SDL_Event* event) {
  if (joystick && (SDL_GetJoystickID(joystick.get()) == event->jdevice.which)) {
    joystick.reset();  // unique_ptrのresetでデストラクタが呼ばれる
  }
}

SDL_AppResult GameManager::handleSdlEvent(SDL_Event* event) {
  return impl->handleSdlEvent(event);
}

SDL_AppResult GameManager::update() { return impl->update(); }

}  // namespace MyGame
