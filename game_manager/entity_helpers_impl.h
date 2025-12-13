#pragma once

#include "entity_helpers.h"

namespace MyGame {

// createRectEntityの実装
inline std::unique_ptr<Entity> createRectEntity(
    int layer, float x, float y, float w, float h, SDL_Color color) {
  auto entity = std::make_unique<Entity>(layer);

  // 座標コンポーネント
  entity->addComponent(std::make_unique<Locator>(x, y));

  // 移動コンポーネント
  entity->addComponent(std::make_unique<VelocityMove>(0.0f, 0.0f));

  // 描画コンポーネント
  entity->addComponent(std::make_unique<RectRenderer>(w, h, color));

  return entity;
}

// createRotateRectEntityの実装
inline std::unique_ptr<Entity> createRotateRectEntity(
    int layer, float x, float y, float w, float h, SDL_Color color,
    float angle, float pivot_x, float pivot_y) {
  auto entity = std::make_unique<Entity>(layer);

  // 座標コンポーネント
  entity->addComponent(std::make_unique<Locator>(x, y));

  // 回転コンポーネント
  entity->addComponent(std::make_unique<Rotater>(angle));

  // 移動コンポーネント
  entity->addComponent(std::make_unique<VelocityMove>(0.0f, 0.0f));

  // 角速度コンポーネント
  entity->addComponent(std::make_unique<AngularVelocity>(0.0f));

  // 回転矩形描画コンポーネント
  entity->addComponent(std::make_unique<RotatedRectRenderer>(w, h, color, pivot_x, pivot_y));

  return entity;
}

// createTextEntity（静的テキスト版）の実装
inline std::unique_ptr<Entity> createTextEntity(
    int layer, float x, float y, const std::string& text,
    SDL_Color color, const UIAnchor* anchor) {
  auto entity = std::make_unique<Entity>(layer);

  // 座標コンポーネント
  entity->addComponent(std::make_unique<Locator>(x, y));

  // テキスト描画コンポーネント
  entity->addComponent(std::make_unique<TextRenderer>(text, color));

  // UIアンカーが指定されている場合は追加
  if (anchor) {
    entity->addComponent(std::make_unique<UIAnchorComponent>(*anchor));
  }

  return entity;
}

// createTextEntity（動的テキスト版）の実装
inline std::unique_ptr<Entity> createTextEntity(
    int layer, float x, float y, std::function<std::string()> text_provider,
    SDL_Color color, const UIAnchor* anchor) {
  auto entity = std::make_unique<Entity>(layer);

  // 座標コンポーネント
  entity->addComponent(std::make_unique<Locator>(x, y));

  // テキスト描画コンポーネント（動的）
  entity->addComponent(std::make_unique<TextRenderer>(text_provider, color));

  // UIアンカーが指定されている場合は追加
  if (anchor) {
    entity->addComponent(std::make_unique<UIAnchorComponent>(*anchor));
  }

  return entity;
}

}  // namespace MyGame
