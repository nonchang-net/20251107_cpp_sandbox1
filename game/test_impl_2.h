#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <iostream>

#include "../game_manager/draw_helper.h"
#include "../game_manager/game_impl.h"
namespace MyGame {

/**
 * @brief テスト用のゲーム実装2
 *
 * ランダムに配置された点群を描画するシンプルな実装です。
 */
class TestImpl2 final : public GameImpl {
 private:
  SDL_Renderer* renderer = nullptr;
  DrawHelper painter;
  std::array<SDL_FPoint, 500> points;

 public:
  TestImpl2(SDL_Renderer* renderer) : renderer(renderer), painter(renderer) {};
  SDL_AppResult handleSdlEvent(SDL_Event*) override;
  SDL_AppResult update() override;
};

// クラス定義完了後にGameImplementation conceptを満たすことを確認
static_assert(GameImplementation<TestImpl2>,
              "TestImpl2 must satisfy GameImplementation concept");

}  // namespace MyGame