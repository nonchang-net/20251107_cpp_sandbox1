#pragma once

#include <SDL3/SDL.h>

namespace MyGame {

// 前方宣言
class Entity;
class Component;
class Locator;
class VelocityMove;
class RectRenderer;

/**
 * @brief 画面端で跳ね返るコンポーネント
 *
 * 画面端に達したら速度を反転させます。
 * Locator、VelocityMove、RectRendererコンポーネントが必要です。
 */
class BounceOnEdge : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param screen_width 画面幅（デフォルト: 640）
   * @param screen_height 画面高さ（デフォルト: 480）
   */
  explicit BounceOnEdge(float screen_width = 640.0f, float screen_height = 480.0f)
      : screen_width_(screen_width), screen_height_(screen_height) {}

  void update(Entity* entity, Uint64 delta_time) override;

  /**
   * @brief 画面サイズを設定
   * @param width 画面幅
   * @param height 画面高さ
   */
  void setScreenSize(float width, float height) {
    screen_width_ = width;
    screen_height_ = height;
  }

 private:
  float screen_width_;
  float screen_height_;
};

}  // namespace MyGame
