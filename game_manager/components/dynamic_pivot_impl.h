#pragma once

#include <cmath>

#include "dynamic_pivot.h"

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
