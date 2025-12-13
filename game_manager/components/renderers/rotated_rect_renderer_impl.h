#pragma once

#include <cmath>

#include "rotated_rect_renderer.h"

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
