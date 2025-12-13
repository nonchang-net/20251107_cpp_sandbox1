#pragma once

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
