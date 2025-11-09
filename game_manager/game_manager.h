#pragma once

#include <SDL3/SDL.h>

#include <iostream>
#include <memory>
#include <string>

#include "../game_constant.h"
#include "game_impl.h"

namespace MyGame {

/**
 * @brief ゲーム実装を管理するテンプレートクラス
 *
 * @tparam GameType GameImplementation conceptを満たす型
 *
 * GameImplementation conceptを使用して、コンパイル時に型チェックを行います。
 * ジョイスティックの管理や、SDL_Eventの委譲を担当します。
 * 
 * note: 現状、無理やりconceptのrequires試すためだけにtemplate書いてるだけになっていて恩恵は特にないけど練習なので気にせずで。
 */
template <typename GameType>
  requires(GameImplementation<GameType>)
class GameManager {
 private:
  std::unique_ptr<SDL_Joystick, decltype(&SDL_CloseJoystick)> joystick{
      nullptr, SDL_CloseJoystick};
  std::unique_ptr<GameType> impl;

  // タイムスケール管理
  float timescale_ = 1.0f;         // 現在のタイムスケール
  float saved_timescale_ = 1.0f;   // ポーズ前のタイムスケールを保存
  bool is_paused_ = false;          // ポーズ状態

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

  /**
   * @brief 現在のタイムスケールを取得します
   * @return float タイムスケール値（1.0 = 100%, 0.5 = 50%, 0.0 = ポーズ）
   */
  float getTimeScale() const { return timescale_; }

  /**
   * @brief タイムスケールを設定し、変更イベントを発火します
   * @param scale タイムスケール値（0.0以上）
   */
  void setTimeScale(float scale) {
    if (scale < 0.0f) scale = 0.0f;

    timescale_ = scale;

    // タイムスケール変更イベントを発火
    SDL_Event event;
    SDL_zero(event);
    event.type = EVENT_TIMESCALE_CHANGED;
    event.user.data1 = this;
    event.user.code = static_cast<Sint32>(timescale_ * 100.0f);  // 整数化して渡す
    SDL_PushEvent(&event);
  }

  /**
   * @brief ポーズ状態をトグルします
   *
   * ポーズ時：現在のタイムスケールを保存してタイムスケールを0に設定し、EVENT_PAUSEを発火
   * アンポーズ時：保存していたタイムスケールを復元し、EVENT_UNPAUSEを発火
   * いずれの場合もEVENT_TIMESCALE_CHANGEDも発火します
   */
  void togglePause() {
    if (is_paused_) {
      // アンポーズ
      is_paused_ = false;
      timescale_ = saved_timescale_;

      // アンポーズイベントを発火
      SDL_Event event;
      SDL_zero(event);
      event.type = EVENT_UNPAUSE;
      event.user.data1 = this;
      SDL_PushEvent(&event);

      // タイムスケール変更イベントも発火
      SDL_zero(event);
      event.type = EVENT_TIMESCALE_CHANGED;
      event.user.data1 = this;
      event.user.code = static_cast<Sint32>(timescale_ * 100.0f);  // 整数化して渡す
      SDL_PushEvent(&event);
    } else {
      // ポーズ
      is_paused_ = true;
      saved_timescale_ = timescale_;
      timescale_ = 0.0f;

      // ポーズイベントを発火
      SDL_Event event;
      SDL_zero(event);
      event.type = EVENT_PAUSE;
      event.user.data1 = this;
      SDL_PushEvent(&event);

      // タイムスケール変更イベントも発火
      SDL_zero(event);
      event.type = EVENT_TIMESCALE_CHANGED;
      event.user.data1 = this;
      event.user.code = static_cast<Sint32>(timescale_ * 100.0f);  // 整数化して渡す
      SDL_PushEvent(&event);
    }
  }

  /**
   * @brief ポーズ状態を取得します
   * @return bool ポーズ中の場合true
   */
  bool isPaused() const { return is_paused_; }
};

}  // namespace MyGame