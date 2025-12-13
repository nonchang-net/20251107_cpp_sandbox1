#ifndef MYGAME_ROTATED_RECT_RENDERER_H_INCLUDED
#define MYGAME_ROTATED_RECT_RENDERER_H_INCLUDED

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

#endif  // MYGAME_ROTATED_RECT_RENDERER_H_INCLUDED

// ========================================================================
// 実装部分（条件付きコンパイル）
// ========================================================================
#if defined(MYGAME_ENTITY_DEFINED) && !defined(MYGAME_ROTATED_RECT_RENDERER_IMPL_INCLUDED)
#define MYGAME_ROTATED_RECT_RENDERER_IMPL_INCLUDED

#include <cmath>

namespace MyGame {

// RotatedRectRendererの実装
inline void RotatedRectRenderer::render(Entity* entity, SDL_Renderer* renderer) {
  // Entityのワールド座標、回転角度、スケールを取得
  auto [world_x, world_y] = entity->getWorldPosition();
  float world_angle = entity->getWorldAngle();
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

  // 角度をラジアンに変換
  float rad = world_angle * (3.14159265358979323846f / 180.0f);
  float cos_a = std::cos(rad);
  float sin_a = std::sin(rad);

  // 矩形の半分のサイズ
  float half_w = scaled_width / 2.0f;
  float half_h = scaled_height / 2.0f;

  // ピボット位置のオフセット（矩形の中心からの相対位置）
  float pivot_offset_x = (pivot_x_ - 0.5f) * scaled_width;
  float pivot_offset_y = (pivot_y_ - 0.5f) * scaled_height;

  // 回転前の4頂点（中心が原点）
  SDL_FPoint local_vertices[4] = {
      {-half_w, -half_h},  // 左上
      {half_w, -half_h},   // 右上
      {half_w, half_h},    // 右下
      {-half_w, half_h}    // 左下
  };

  // 回転した頂点を計算
  SDL_FPoint vertices[4];
  for (int i = 0; i < 4; i++) {
    // 頂点のピボットからの相対位置
    float x = local_vertices[i].x - pivot_offset_x;
    float y = local_vertices[i].y - pivot_offset_y;

    // 回転
    float rotated_x = x * cos_a - y * sin_a;
    float rotated_y = x * sin_a + y * cos_a;

    // ピボット位置に戻して画面座標に移動
    vertices[i].x = rotated_x + pivot_offset_x + screen_x;
    vertices[i].y = rotated_y + pivot_offset_y + screen_y;
  }

  // SDL_RenderGeometryで描画（塗りつぶし）
  SDL_Vertex sdl_vertices[4];
  for (int i = 0; i < 4; i++) {
    sdl_vertices[i].position = vertices[i];
    sdl_vertices[i].color.r = color_.r;
    sdl_vertices[i].color.g = color_.g;
    sdl_vertices[i].color.b = color_.b;
    sdl_vertices[i].color.a = color_.a;
    sdl_vertices[i].tex_coord = {0.0f, 0.0f};  // テクスチャなし
  }

  // 2つの三角形で矩形を描画
  int indices[6] = {0, 1, 2, 2, 3, 0};
  SDL_RenderGeometry(renderer, nullptr, sdl_vertices, 4, indices, 6);
}

}  // namespace MyGame

#endif  // MYGAME_ENTITY_DEFINED && !MYGAME_ROTATED_RECT_RENDERER_IMPL_INCLUDED
