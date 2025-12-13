#ifndef MYGAME_BOUNCE_ON_EDGE_H_INCLUDED
#define MYGAME_BOUNCE_ON_EDGE_H_INCLUDED

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

#endif  // MYGAME_BOUNCE_ON_EDGE_H_INCLUDED

// ========================================================================
// 実装部分（条件付きコンパイル）
// ========================================================================
#if defined(MYGAME_ENTITY_DEFINED) && !defined(MYGAME_BOUNCE_ON_EDGE_IMPL_INCLUDED)
#define MYGAME_BOUNCE_ON_EDGE_IMPL_INCLUDED

namespace MyGame {

// BounceOnEdgeの実装
inline void BounceOnEdge::update(Entity* entity, Uint64 delta_time) {
  auto* locator = entity->getComponent<Locator>();
  auto* velocity = entity->getComponent<VelocityMove>();
  auto* renderer = entity->getComponent<RectRenderer>();

  if (!locator || !velocity || !renderer) {
    return;
  }

  auto [x, y] = locator->getPosition();
  auto [w, h] = renderer->getSize();
  auto [vx, vy] = velocity->getVelocity();

  bool changed = false;
  if (x < 0 || x + w > screen_width_) {
    vx = -vx;
    changed = true;
  }
  if (y < 0 || y + h > screen_height_) {
    vy = -vy;
    changed = true;
  }

  if (changed) {
    velocity->setVelocity(vx, vy);
  }
}

}  // namespace MyGame

#endif  // MYGAME_ENTITY_DEFINED && !MYGAME_BOUNCE_ON_EDGE_IMPL_INCLUDED
