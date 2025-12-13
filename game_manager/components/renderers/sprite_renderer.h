#pragma once

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
