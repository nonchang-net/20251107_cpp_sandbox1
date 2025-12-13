#pragma once

#include "text_renderer.h"

namespace MyGame {

// TextRendererの実装
inline void TextRenderer::update(Entity* entity, Uint64 delta_time) {
  // 動的テキストの場合、毎フレーム更新
  if (text_provider_) {
    text_ = text_provider_();
  }
}

inline void TextRenderer::render(Entity* entity, SDL_Renderer* renderer) {
  float screen_x, screen_y;

  // UIAnchorコンポーネントがあるかチェック
  if (auto* ui_anchor = entity->getComponent<UIAnchorComponent>()) {
    // UI要素として扱う：カメラの影響を受けず、画面座標で描画
    auto* camera = entity->getRenderCamera();
    float viewport_width = 640.0f;   // デフォルト値
    float viewport_height = 480.0f;  // デフォルト値

    // カメラからビューポートサイズを取得
    if (camera) {
      // Camera2Dからビューポートサイズを取得（将来的に実装）
      // 現在はデフォルト値を使用
    }

    // アンカー座標を計算
    auto [anchor_x, anchor_y] = ui_anchor->calculateAnchorPosition(
        viewport_width, viewport_height);

    // Locatorのオフセットを取得
    auto [offset_x, offset_y] = entity->getLocalPosition();

    // 画面座標 = アンカー座標 + オフセット
    screen_x = anchor_x + offset_x;
    screen_y = anchor_y + offset_y;
  } else {
    // ゲーム要素として扱う：ワールド座標 + カメラ変換
    auto [world_x, world_y] = entity->getWorldPosition();

    // カメラを使用してワールド座標から画面座標に変換
    if (auto* camera = entity->getRenderCamera()) {
      auto [sx, sy] = camera->worldToScreen(world_x, world_y);
      screen_x = sx;
      screen_y = sy;
    } else {
      screen_x = world_x;
      screen_y = world_y;
    }
  }

  // テキストを描画
  SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, color_.a);
  SDL_RenderDebugText(renderer, screen_x, screen_y, text_.c_str());
}

}  // namespace MyGame
