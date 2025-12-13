#pragma once

#include "sprite_animator.h"

namespace MyGame {

// SpriteAnimatorの実装
inline void SpriteAnimator::update(Entity* entity, Uint64 delta_time) {
  if (frames_.empty()) return;

  // SpriteRendererコンポーネントを取得
  auto* sprite_renderer = entity->getComponent<SpriteRenderer>();
  if (!sprite_renderer) return;

  // タイマーを進める
  timer_ += delta_time;

  // フレーム切り替えが必要かチェック
  if (timer_ >= frame_duration_) {
    timer_ -= frame_duration_;

    // 次のフレームへ
    current_frame_ = (current_frame_ + 1) % frames_.size();

    // SpriteRendererのタイルを更新
    auto [tile_x, tile_y] = frames_[current_frame_];
    sprite_renderer->setTile(tile_x, tile_y);
  }
}

// DirectionalSpriteAnimatorの実装
inline void DirectionalSpriteAnimator::update(Entity* entity, Uint64 delta_time) {
  // DirectionComponentを取得
  auto* direction_comp = entity->getComponent<DirectionComponent>();
  if (!direction_comp) return;

  Direction new_direction = direction_comp->getDirection();

  // 向きが変わった場合のみ、フレームを切り替える
  if (new_direction != current_direction_) {
    current_direction_ = new_direction;

    auto* animator = entity->getComponent<SpriteAnimator>();
    auto* sprite_renderer = entity->getComponent<SpriteRenderer>();
    if (!animator) return;

    // 向きに応じてフレームを設定
    switch (new_direction) {
      case Direction::Down:
        animator->setFrames(down_frames_);
        if (sprite_renderer && !down_frames_.empty()) {
          sprite_renderer->setFlipHorizontal(false);
          // 即座に新しいアニメーションの最初のフレームを表示
          auto [tile_x, tile_y] = down_frames_[0];
          sprite_renderer->setTile(tile_x, tile_y);
        }
        break;
      case Direction::Up:
        animator->setFrames(up_frames_);
        if (sprite_renderer && !up_frames_.empty()) {
          sprite_renderer->setFlipHorizontal(false);
          // 即座に新しいアニメーションの最初のフレームを表示
          auto [tile_x, tile_y] = up_frames_[0];
          sprite_renderer->setTile(tile_x, tile_y);
        }
        break;
      case Direction::Right:
        animator->setFrames(right_frames_);
        if (sprite_renderer && !right_frames_.empty()) {
          sprite_renderer->setFlipHorizontal(false);
          // 即座に新しいアニメーションの最初のフレームを表示
          auto [tile_x, tile_y] = right_frames_[0];
          sprite_renderer->setTile(tile_x, tile_y);
        }
        break;
      case Direction::Left:
        if (left_frames_.empty()) {
          // 左向きフレームが未定義の場合、右向きを左右反転
          animator->setFrames(right_frames_);
          if (sprite_renderer && !right_frames_.empty()) {
            sprite_renderer->setFlipHorizontal(true);
            // 即座に新しいアニメーションの最初のフレームを表示
            auto [tile_x, tile_y] = right_frames_[0];
            sprite_renderer->setTile(tile_x, tile_y);
          }
        } else {
          // 左向きフレームが定義されている場合
          animator->setFrames(left_frames_);
          if (sprite_renderer && !left_frames_.empty()) {
            sprite_renderer->setFlipHorizontal(false);
            // 即座に新しいアニメーションの最初のフレームを表示
            auto [tile_x, tile_y] = left_frames_[0];
            sprite_renderer->setTile(tile_x, tile_y);
          }
        }
        break;
    }
  }
}

}  // namespace MyGame
