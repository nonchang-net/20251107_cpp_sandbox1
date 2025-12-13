#ifndef MYGAME_SPRITE_ANIMATOR_H_INCLUDED
#define MYGAME_SPRITE_ANIMATOR_H_INCLUDED

#include <SDL3/SDL.h>

#include <utility>
#include <vector>

namespace MyGame {

// 前方宣言
class Entity;
class Component;
class SpriteRenderer;
class DirectionComponent;
enum class Direction;

/**
 * @brief スプライトアニメーションを管理するコンポーネント
 *
 * フレームリストを順に切り替えてアニメーションします。
 * SpriteRendererコンポーネントが必要です。
 */
class SpriteAnimator : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param frames アニメーションフレームのリスト（タイルのグリッド座標）
   * @param frame_duration 1フレームの表示時間（ミリ秒）
   */
  SpriteAnimator(const std::vector<std::pair<int, int>>& frames,
                 Uint64 frame_duration = 500)
      : frames_(frames), frame_duration_(frame_duration), current_frame_(0),
        timer_(0) {}

  void update(Entity* entity, Uint64 delta_time) override;

  /**
   * @brief フレームリストを設定
   * @param frames 新しいフレームリスト
   */
  void setFrames(const std::vector<std::pair<int, int>>& frames) {
    frames_ = frames;
    current_frame_ = 0;
    timer_ = 0;
  }

  /**
   * @brief フレーム表示時間を設定
   * @param duration 1フレームの表示時間（ミリ秒）
   */
  void setFrameDuration(Uint64 duration) { frame_duration_ = duration; }

  /**
   * @brief 現在のフレーム番号を取得
   * @return フレーム番号
   */
  size_t getCurrentFrame() const { return current_frame_; }

 private:
  std::vector<std::pair<int, int>> frames_;  // フレームリスト（タイル座標）
  Uint64 frame_duration_;                    // 1フレームの表示時間
  size_t current_frame_;                     // 現在のフレーム番号
  Uint64 timer_;                             // タイマー
};

/**
 * @brief 向きごとのスプライトフレームセットを管理するコンポーネント
 *
 * DirectionComponentの向きに応じて、SpriteAnimatorとSpriteRendererを自動更新します。
 * DirectionComponent、SpriteAnimator、SpriteRendererコンポーネントが必要です。
 */
class DirectionalSpriteAnimator : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param down_frames 下向きのフレームリスト
   * @param up_frames 上向きのフレームリスト
   * @param right_frames 右向きのフレームリスト
   * @param left_frames 左向きのフレームリスト（空の場合は右向きを左右反転）
   */
  DirectionalSpriteAnimator(
      const std::vector<std::pair<int, int>>& down_frames,
      const std::vector<std::pair<int, int>>& up_frames,
      const std::vector<std::pair<int, int>>& right_frames,
      const std::vector<std::pair<int, int>>& left_frames = {})
      : down_frames_(down_frames), up_frames_(up_frames),
        right_frames_(right_frames), left_frames_(left_frames),
        current_direction_(Direction::Down) {}

  void update(Entity* entity, Uint64 delta_time) override;

  /**
   * @brief 各向きのフレームリストを設定
   */
  void setDownFrames(const std::vector<std::pair<int, int>>& frames) {
    down_frames_ = frames;
  }
  void setUpFrames(const std::vector<std::pair<int, int>>& frames) {
    up_frames_ = frames;
  }
  void setRightFrames(const std::vector<std::pair<int, int>>& frames) {
    right_frames_ = frames;
  }
  void setLeftFrames(const std::vector<std::pair<int, int>>& frames) {
    left_frames_ = frames;
  }

 private:
  std::vector<std::pair<int, int>> down_frames_;   // 下向きフレーム
  std::vector<std::pair<int, int>> up_frames_;     // 上向きフレーム
  std::vector<std::pair<int, int>> right_frames_;  // 右向きフレーム
  std::vector<std::pair<int, int>> left_frames_;   // 左向きフレーム（空なら右向きを反転）
  Direction current_direction_;  // 前回の向き（変化検出用）
};

}  // namespace MyGame

#endif  // MYGAME_SPRITE_ANIMATOR_H_INCLUDED

// ========================================================================
// 実装部分（条件付きコンパイル）
// ========================================================================
#if defined(MYGAME_ENTITY_DEFINED) && !defined(MYGAME_SPRITE_ANIMATOR_IMPL_INCLUDED)
#define MYGAME_SPRITE_ANIMATOR_IMPL_INCLUDED

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
          auto [tile_x, tile_y] = down_frames_[0];
          sprite_renderer->setTile(tile_x, tile_y);
        }
        break;
      case Direction::Up:
        animator->setFrames(up_frames_);
        if (sprite_renderer && !up_frames_.empty()) {
          sprite_renderer->setFlipHorizontal(false);
          auto [tile_x, tile_y] = up_frames_[0];
          sprite_renderer->setTile(tile_x, tile_y);
        }
        break;
      case Direction::Right:
        animator->setFrames(right_frames_);
        if (sprite_renderer && !right_frames_.empty()) {
          sprite_renderer->setFlipHorizontal(false);
          auto [tile_x, tile_y] = right_frames_[0];
          sprite_renderer->setTile(tile_x, tile_y);
        }
        break;
      case Direction::Left:
        if (left_frames_.empty()) {
          animator->setFrames(right_frames_);
          if (sprite_renderer && !right_frames_.empty()) {
            sprite_renderer->setFlipHorizontal(true);
            auto [tile_x, tile_y] = right_frames_[0];
            sprite_renderer->setTile(tile_x, tile_y);
          }
        } else {
          animator->setFrames(left_frames_);
          if (sprite_renderer && !left_frames_.empty()) {
            sprite_renderer->setFlipHorizontal(false);
            auto [tile_x, tile_y] = left_frames_[0];
            sprite_renderer->setTile(tile_x, tile_y);
          }
        }
        break;
    }
  }
}

}  // namespace MyGame

#endif  // MYGAME_ENTITY_DEFINED && !MYGAME_SPRITE_ANIMATOR_IMPL_INCLUDED
