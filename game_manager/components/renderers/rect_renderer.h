#pragma once

#include <SDL3/SDL.h>

#include <utility>

namespace MyGame {

// 前方宣言
class Entity;
class Component;
class Camera2D;

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

}  // namespace MyGame
