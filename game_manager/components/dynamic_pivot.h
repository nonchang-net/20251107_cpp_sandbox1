#ifndef MYGAME_DYNAMIC_PIVOT_H_INCLUDED
#define MYGAME_DYNAMIC_PIVOT_H_INCLUDED

#include <SDL3/SDL.h>

namespace MyGame {

// 前方宣言
class Entity;
class Component;
class RotatedRectRenderer;

/**
 * @brief ピボットポイントを動的に変更するコンポーネント
 *
 * RotatedRectRendererコンポーネントのピボット位置を周期的に変更します。
 * RotatedRectRendererコンポーネントが必要です。
 */
class DynamicPivot : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param period_ms 周期（ミリ秒）
   */
  explicit DynamicPivot(Uint64 period_ms = 2000)
      : period_(period_ms), timer_(0) {}

  void update(Entity* entity, Uint64 delta_time) override;

  /**
   * @brief 周期を設定
   * @param period_ms 周期（ミリ秒）
   */
  void setPeriod(Uint64 period_ms) { period_ = period_ms; }

  /**
   * @brief 周期を取得
   * @return 周期（ミリ秒）
   */
  Uint64 getPeriod() const { return period_; }

 private:
  Uint64 period_;
  Uint64 timer_;
};

}  // namespace MyGame

#endif  // MYGAME_DYNAMIC_PIVOT_H_INCLUDED

// ========================================================================
// 実装部分（条件付きコンパイル）
// ========================================================================
#if defined(MYGAME_ENTITY_DEFINED) && !defined(MYGAME_DYNAMIC_PIVOT_IMPL_INCLUDED)
#define MYGAME_DYNAMIC_PIVOT_IMPL_INCLUDED

#include <cmath>

namespace MyGame {

// DynamicPivotの実装
inline void DynamicPivot::update(Entity* entity, Uint64 delta_time) {
  auto* renderer = entity->getComponent<RotatedRectRenderer>();
  if (!renderer) return;

  // ピボットを周期的に変更（円運動させる）
  timer_ += delta_time;
  float t = timer_ / static_cast<float>(period_);
  float pivot_x = 0.5f + 0.4f * std::cos(t);  // 0.1～0.9の範囲
  float pivot_y = 0.5f + 0.4f * std::sin(t);  // 0.1～0.9の範囲
  renderer->setPivot(pivot_x, pivot_y);
}

}  // namespace MyGame

#endif  // MYGAME_ENTITY_DEFINED && !MYGAME_DYNAMIC_PIVOT_IMPL_INCLUDED
