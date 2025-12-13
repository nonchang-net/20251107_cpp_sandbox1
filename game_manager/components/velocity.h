#pragma once

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
