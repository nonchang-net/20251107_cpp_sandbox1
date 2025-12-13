#pragma once

#include "sprite_renderer.h"

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
