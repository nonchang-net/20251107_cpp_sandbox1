#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace MyGame {

// 前方宣言
class Entity;

/**
 * @brief コンポーネントの基底クラス
 *
 * Entityに付与される機能の単位です。
 * 座標、回転、描画などの機能を個別のコンポーネントとして実装します。
 */
class Component {
 public:
  virtual ~Component() = default;

  /**
   * @brief コンポーネントの更新処理
   * @param entity このコンポーネントが所属するEntity
   * @param delta_time 前フレームからの経過時間（ミリ秒）
   */
  virtual void update(Entity* entity, Uint64 delta_time) {}

  /**
   * @brief コンポーネントの描画処理
   * @param entity このコンポーネントが所属するEntity
   * @param renderer SDLレンダラー
   */
  virtual void render(Entity* entity, SDL_Renderer* renderer) {}
};

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

/**
 * @brief 矩形を描画するコンポーネント
 *
 * 指定されたサイズと色で矩形を描画します。
 * Entityのワールド座標・回転・スケールを考慮して描画されます。
 */
class RectRenderer : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param width 幅
   * @param height 高さ
   * @param color 色
   */
  RectRenderer(float width, float height, SDL_Color color)
      : width_(width), height_(height), color_(color) {}

  void render(Entity* entity, SDL_Renderer* renderer) override;

  /**
   * @brief サイズを設定
   * @param width 幅
   * @param height 高さ
   */
  void setSize(float width, float height) {
    width_ = width;
    height_ = height;
  }

  /**
   * @brief サイズを取得
   * @return {width, height}
   */
  std::pair<float, float> getSize() const { return {width_, height_}; }

  /**
   * @brief 色を設定
   * @param color 色
   */
  void setColor(SDL_Color color) { color_ = color; }

  /**
   * @brief 色を取得
   * @return 色
   */
  SDL_Color getColor() const { return color_; }

 private:
  float width_, height_;
  SDL_Color color_;
};

/**
 * @brief 回転する矩形を描画するコンポーネント
 *
 * ピボットポイントを考慮して回転した矩形を描画します。
 * Entityのワールド座標・回転・スケールを考慮して描画されます。
 */
class RotatedRectRenderer : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param width 幅
   * @param height 高さ
   * @param color 色
   * @param pivot_x X方向のピボット位置（0.0～1.0、0.0=左端、0.5=中央、1.0=右端）
   * @param pivot_y Y方向のピボット位置（0.0～1.0、0.0=上端、0.5=中央、1.0=下端）
   */
  RotatedRectRenderer(float width, float height, SDL_Color color,
                      float pivot_x = 0.5f, float pivot_y = 0.5f)
      : width_(width), height_(height), color_(color),
        pivot_x_(pivot_x), pivot_y_(pivot_y) {}

  void render(Entity* entity, SDL_Renderer* renderer) override;

  /**
   * @brief サイズを設定
   * @param width 幅
   * @param height 高さ
   */
  void setSize(float width, float height) {
    width_ = width;
    height_ = height;
  }

  /**
   * @brief サイズを取得
   * @return {width, height}
   */
  std::pair<float, float> getSize() const { return {width_, height_}; }

  /**
   * @brief 色を設定
   * @param color 色
   */
  void setColor(SDL_Color color) { color_ = color; }

  /**
   * @brief 色を取得
   * @return 色
   */
  SDL_Color getColor() const { return color_; }

  /**
   * @brief 回転の原点（ピボットポイント）を設定
   * @param pivot_x X方向のピボット位置（0.0～1.0）
   * @param pivot_y Y方向のピボット位置（0.0～1.0）
   */
  void setPivot(float pivot_x, float pivot_y) {
    pivot_x_ = pivot_x;
    pivot_y_ = pivot_y;
  }

  /**
   * @brief 回転の原点（ピボットポイント）を取得
   * @return {pivot_x, pivot_y}
   */
  std::pair<float, float> getPivot() const { return {pivot_x_, pivot_y_}; }

 private:
  float width_, height_;
  SDL_Color color_;
  float pivot_x_, pivot_y_;  // 回転の原点（0.0～1.0）
};

}  // namespace MyGame
