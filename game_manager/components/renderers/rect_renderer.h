#ifndef MYGAME_RECT_RENDERER_H_INCLUDED
#define MYGAME_RECT_RENDERER_H_INCLUDED

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

#endif  // MYGAME_RECT_RENDERER_H_INCLUDED

// ========================================================================
// 実装部分（条件付きコンパイル）
// ========================================================================
#if defined(MYGAME_ENTITY_DEFINED) && !defined(MYGAME_RECT_RENDERER_IMPL_INCLUDED)
#define MYGAME_RECT_RENDERER_IMPL_INCLUDED

namespace MyGame {

// RectRendererの実装
inline void RectRenderer::render(Entity* entity, SDL_Renderer* renderer) {
  // Entityのワールド座標とスケールを取得
  auto [world_x, world_y] = entity->getWorldPosition();
  auto [scale_x, scale_y] = entity->getWorldScale();

  // カメラを使用してワールド座標から画面座標に変換
  float screen_x = world_x;
  float screen_y = world_y;
  if (auto* camera = entity->getRenderCamera()) {
    auto [sx, sy] = camera->worldToScreen(world_x, world_y);
    screen_x = sx;
    screen_y = sy;
  }

  // スケールを適用したサイズ
  float scaled_width = width_ * scale_x;
  float scaled_height = height_ * scale_y;

  // 矩形を描画（左上座標基準）
  SDL_FRect rect{screen_x, screen_y, scaled_width, scaled_height};

  SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, color_.a);
  SDL_RenderFillRect(renderer, &rect);
}

}  // namespace MyGame

#endif  // MYGAME_ENTITY_DEFINED && !MYGAME_RECT_RENDERER_IMPL_INCLUDED
