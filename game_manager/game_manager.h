#pragma once

#include <SDL3/SDL.h>

#include <iostream>
#include <memory>
#include <string>

#include "game_impl.h"

namespace MyGame {

/**
 * @brief ゲーム実装を管理するテンプレートクラス
 *
 * @tparam GameType GameImplementation conceptを満たす型
 *
 * GameImplementation conceptを使用して、コンパイル時に型チェックを行います。
 * ジョイスティックの管理や、SDL_Eventの委譲を担当します。
 */
template <typename GameType>
  requires(GameImplementation<GameType>)
class GameManager {
 private:
  std::unique_ptr<SDL_Joystick, decltype(&SDL_CloseJoystick)> joystick{
      nullptr, SDL_CloseJoystick};
  std::unique_ptr<GameType> impl;

 public:
  /**
   * @brief GameManagerを構築します
   * @param impl ゲーム実装のインスタンス
   */
  explicit GameManager(std::unique_ptr<GameType> impl)
      : impl(std::move(impl)) {}

  /**
   * @brief デストラクタ
   * unique_ptrが自動的にリソースを解放します
   */
  ~GameManager() = default;

  /**
   * @brief ゲームの更新処理を実行します
   * @return SDL_AppResult 実行結果
   */
  SDL_AppResult update() { return impl->update(); }

  /**
   * @brief ジョイスティックを追加します
   * @param event SDL_Event
   */
  void addJoystick(SDL_Event* event) {
    if (!joystick) {
      joystick.reset(SDL_OpenJoystick(event->jdevice.which));
      if (!joystick) {
        SDL_Log("Failed to open joystick ID %u: %s",
                (unsigned int)event->jdevice.which, SDL_GetError());
      }
    }
  }

  /**
   * @brief ジョイスティックを削除します
   * @param event SDL_Event
   */
  void removeJoystick(SDL_Event* event) {
    if (joystick &&
        (SDL_GetJoystickID(joystick.get()) == event->jdevice.which)) {
      joystick.reset();
    }
  }

  /**
   * @brief SDL_Eventを処理します
   * @param event SDL_Event
   * @return SDL_AppResult 実行結果
   */
  SDL_AppResult handleSdlEvent(SDL_Event* event) {
    return impl->handleSdlEvent(event);
  }
};

}  // namespace MyGame