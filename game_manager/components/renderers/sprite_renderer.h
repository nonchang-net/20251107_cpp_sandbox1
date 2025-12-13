#ifndef MYGAME_SPRITE_RENDERER_H_INCLUDED
#define MYGAME_SPRITE_RENDERER_H_INCLUDED

#include <SDL3/SDL.h>

#include <utility>

namespace MyGame {

// 前方宣言
class Entity;
class Component;

/**
 * @brief スプライトシートから特定のタイルを描画するコンポーネント
 *
 * テクスチャをグリッド状のタイルに分割し、指定した座標のタイルを描画します。
 * Entityのワールド座標・スケールを考慮して描画されます。
 * 左右反転機能をサポートします。
 */
class SpriteRenderer : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param texture スプライトシートのテクスチャ
   * @param tile_size タイル1つのサイズ（ピクセル）
   * @param tile_x 描画するタイルのX座標（グリッド座標）
   * @param tile_y 描画するタイルのY座標（グリッド座標）
   */
  SpriteRenderer(SDL_Texture* texture, int tile_size, int tile_x = 0,
                 int tile_y = 0, bool flip_horizontal = false)
      : texture_(texture), tile_size_(tile_size), tile_x_(tile_x),
        tile_y_(tile_y), flip_horizontal_(flip_horizontal) {}

  void render(Entity* entity, SDL_Renderer* renderer) override;

  /**
   * @brief 描画するタイルを設定
   * @param tile_x タイルのX座標（グリッド座標）
   * @param tile_y タイルのY座標（グリッド座標）
   */
  void setTile(int tile_x, int tile_y) {
    tile_x_ = tile_x;
    tile_y_ = tile_y;
  }

  /**
   * @brief 描画するタイルを取得
   * @return {tile_x, tile_y}
   */
  std::pair<int, int> getTile() const { return {tile_x_, tile_y_}; }

  /**
   * @brief テクスチャを設定
   * @param texture 新しいテクスチャ
   */
  void setTexture(SDL_Texture* texture) { texture_ = texture; }

  /**
   * @brief テクスチャを取得
   * @return テクスチャ
   */
  SDL_Texture* getTexture() const { return texture_; }

  /**
   * @brief 左右反転を設定
   * @param flip 反転するかどうか
   */
  void setFlipHorizontal(bool flip) { flip_horizontal_ = flip; }

  /**
   * @brief 左右反転を取得
   * @return 反転しているかどうか
   */
  bool isFlipHorizontal() const { return flip_horizontal_; }

 private:
  SDL_Texture* texture_;     // スプライトシートのテクスチャ
  int tile_size_;            // タイル1つのサイズ
  int tile_x_, tile_y_;      // 描画するタイルのグリッド座標
  bool flip_horizontal_;     // 左右反転フラグ
};

}  // namespace MyGame

#endif  // MYGAME_SPRITE_RENDERER_H_INCLUDED

// ========================================================================
// 実装部分（条件付きコンパイル）
// ========================================================================
#if defined(MYGAME_ENTITY_DEFINED) && !defined(MYGAME_SPRITE_RENDERER_IMPL_INCLUDED)
#define MYGAME_SPRITE_RENDERER_IMPL_INCLUDED

namespace MyGame {

// SpriteRendererの実装
inline void SpriteRenderer::render(Entity* entity, SDL_Renderer* renderer) {
  if (!texture_) return;

  // ワールド座標を取得
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

  // スプライトシート上のソース矩形を計算
  SDL_FRect src_rect;
  src_rect.x = static_cast<float>(tile_x_ * tile_size_);
  src_rect.y = static_cast<float>(tile_y_ * tile_size_);
  src_rect.w = static_cast<float>(tile_size_);
  src_rect.h = static_cast<float>(tile_size_);

  // 描画先の矩形を計算（スケール適用）
  SDL_FRect dst_rect;
  dst_rect.x = screen_x;
  dst_rect.y = screen_y;
  dst_rect.w = tile_size_ * scale_x;
  dst_rect.h = tile_size_ * scale_y;

  // テクスチャを描画（左右反転対応）
  if (flip_horizontal_) {
    // 左右反転する場合、SDL_RenderTextureRotatedでSDL_FLIP_HORIZONTALを使用
    SDL_RenderTextureRotated(renderer, texture_, &src_rect, &dst_rect,
                              0.0, nullptr, SDL_FLIP_HORIZONTAL);
  } else {
    SDL_RenderTexture(renderer, texture_, &src_rect, &dst_rect);
  }
}

}  // namespace MyGame

#endif  // MYGAME_ENTITY_DEFINED && !MYGAME_SPRITE_RENDERER_IMPL_INCLUDED
