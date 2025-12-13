#pragma once

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
