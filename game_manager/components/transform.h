#pragma once

#include <SDL3/SDL.h>

#include <utility>

namespace MyGame {

// 前方宣言
class Entity;
class Component;

/**
 * @brief 2D座標を保持するコンポーネント
 *
 * Entityのローカル座標（親からの相対位置）を管理します。
 * このコンポーネントがない場合、座標は(0, 0)として扱われます。
 */
class Locator : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param x X座標（親からの相対位置）
   * @param y Y座標（親からの相対位置）
   */
  Locator(float x = 0.0f, float y = 0.0f) : x_(x), y_(y) {}

  /**
   * @brief 座標を設定
   * @param x X座標
   * @param y Y座標
   */
  void setPosition(float x, float y) {
    x_ = x;
    y_ = y;
  }

  /**
   * @brief 座標を取得
   * @return {x, y}
   */
  std::pair<float, float> getPosition() const { return {x_, y_}; }

  /**
   * @brief X座標を取得
   * @return X座標
   */
  float getX() const { return x_; }

  /**
   * @brief Y座標を取得
   * @return Y座標
   */
  float getY() const { return y_; }

 private:
  float x_, y_;
};

/**
 * @brief 2D回転を保持するコンポーネント
 *
 * Entityのローカル回転角度（親からの相対角度）を管理します。
 * このコンポーネントがない場合、回転角度は0として扱われます。
 */
class Rotater : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param angle 回転角度（度数法）
   */
  explicit Rotater(float angle = 0.0f) : angle_(angle) {}

  /**
   * @brief 回転角度を設定
   * @param angle 回転角度（度数法）
   */
  void setAngle(float angle) { angle_ = angle; }

  /**
   * @brief 回転角度を取得
   * @return 回転角度（度数法）
   */
  float getAngle() const { return angle_; }

 private:
  float angle_;
};

/**
 * @brief 2Dスケールを保持するコンポーネント
 *
 * EntityのX方向・Y方向のスケールを管理します。
 * このコンポーネントがない場合、スケールは(1.0, 1.0)として扱われます。
 */
class Scaler : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param scale_x X方向のスケール
   * @param scale_y Y方向のスケール
   */
  Scaler(float scale_x = 1.0f, float scale_y = 1.0f)
      : scale_x_(scale_x), scale_y_(scale_y) {}

  /**
   * @brief スケールを設定
   * @param scale_x X方向のスケール
   * @param scale_y Y方向のスケール
   */
  void setScale(float scale_x, float scale_y) {
    scale_x_ = scale_x;
    scale_y_ = scale_y;
  }

  /**
   * @brief スケールを取得
   * @return {scale_x, scale_y}
   */
  std::pair<float, float> getScale() const { return {scale_x_, scale_y_}; }

  /**
   * @brief X方向のスケールを取得
   * @return X方向のスケール
   */
  float getScaleX() const { return scale_x_; }

  /**
   * @brief Y方向のスケールを取得
   * @return Y方向のスケール
   */
  float getScaleY() const { return scale_y_; }

 private:
  float scale_x_, scale_y_;
};

}  // namespace MyGame
