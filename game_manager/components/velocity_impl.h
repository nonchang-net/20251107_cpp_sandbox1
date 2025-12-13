#pragma once

#include "velocity.h"

namespace MyGame {

// VelocityMoveの実装
inline void VelocityMove::update(Entity* entity, Uint64 delta_time) {
  // Locatorコンポーネントを取得して座標を更新
  if (auto* locator = entity->getComponent<Locator>()) {
    auto [x, y] = locator->getPosition();
    // delta_timeをミリ秒→秒に変換して速度を適用
    // 速度の単位は「ピクセル/秒」
    float dt_sec = delta_time / 1000.0f;
    locator->setPosition(x + velocity_x_ * dt_sec, y + velocity_y_ * dt_sec);
  }
}

// AngularVelocityの実装
inline void AngularVelocity::update(Entity* entity, Uint64 delta_time) {
  // Rotaterコンポーネントを取得して角度を更新
  if (auto* rotater = entity->getComponent<Rotater>()) {
    float angle = rotater->getAngle();
    // delta_timeをミリ秒→秒に変換して角速度を適用
    angle += angular_velocity_ * (delta_time / 1000.0f);

    // 角度を0-360度に正規化
    while (angle >= 360.0f) angle -= 360.0f;
    while (angle < 0.0f) angle += 360.0f;

    rotater->setAngle(angle);
  }
}

}  // namespace MyGame
