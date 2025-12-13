#pragma once

#include <SDL3/SDL.h>

#include <utility>

namespace MyGame {

// 前方宣言
class Entity;
class Component;

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
