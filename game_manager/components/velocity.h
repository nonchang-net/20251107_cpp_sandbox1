#ifndef MYGAME_VELOCITY_H_INCLUDED
#define MYGAME_VELOCITY_H_INCLUDED

#include <SDL3/SDL.h>

#include <utility>

namespace MyGame {

// 前方宣言
class Entity;
class Component;
class Locator;
class Rotater;

/**
 * @brief 速度による移動を行うコンポーネント
 *
 * 毎フレーム、Locatorコンポーネントの座標を速度分だけ移動させます。
 * Locatorコンポーネントが必要です。
 */
class VelocityMove : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param vx X方向の速度
   * @param vy Y方向の速度
   */
  VelocityMove(float vx = 0.0f, float vy = 0.0f)
      : velocity_x_(vx), velocity_y_(vy) {}

  void update(Entity* entity, Uint64 delta_time) override;

  /**
   * @brief 速度を設定
   * @param vx X方向の速度
   * @param vy Y方向の速度
   */
  void setVelocity(float vx, float vy) {
    velocity_x_ = vx;
    velocity_y_ = vy;
  }

  /**
   * @brief 速度を取得
   * @return {vx, vy}
   */
  std::pair<float, float> getVelocity() const {
    return {velocity_x_, velocity_y_};
  }

 private:
  float velocity_x_, velocity_y_;
};

/**
 * @brief 角速度による回転を行うコンポーネント
 *
 * 毎フレーム、Rotaterコンポーネントの角度を角速度分だけ回転させます。
 * Rotaterコンポーネントが必要です。
 */
class AngularVelocity : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param angular_vel 角速度（度/秒）
   */
  explicit AngularVelocity(float angular_vel = 0.0f)
      : angular_velocity_(angular_vel) {}

  void update(Entity* entity, Uint64 delta_time) override;

  /**
   * @brief 角速度を設定
   * @param angular_vel 角速度（度/秒）
   */
  void setAngularVelocity(float angular_vel) { angular_velocity_ = angular_vel; }

  /**
   * @brief 角速度を取得
   * @return 角速度（度/秒）
   */
  float getAngularVelocity() const { return angular_velocity_; }

 private:
  float angular_velocity_;
};

}  // namespace MyGame

#endif  // MYGAME_VELOCITY_H_INCLUDED

// ========================================================================
// 実装部分（条件付きコンパイル）
// Entityの完全な定義が必要なため、entity_manager.hの後でインクルードされる
// ========================================================================
#if defined(MYGAME_ENTITY_DEFINED) && !defined(MYGAME_VELOCITY_IMPL_INCLUDED)
#define MYGAME_VELOCITY_IMPL_INCLUDED

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

#endif  // MYGAME_ENTITY_DEFINED && !MYGAME_VELOCITY_IMPL_INCLUDED
