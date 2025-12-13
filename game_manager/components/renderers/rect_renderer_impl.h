#pragma once

#include "rect_renderer.h"

// Entityとカメラの完全な定義が必要
// これらはentity_manager.hで定義されているが、循環参照を避けるため
// この実装ファイルは最後にインクルードされることを前提とする

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
