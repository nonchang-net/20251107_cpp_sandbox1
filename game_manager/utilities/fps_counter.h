#pragma once

#include <SDL3/SDL.h>

#include <deque>

namespace MyGame::Utilities {

/**
 * @brief FPS（フレームレート）を計測するクラス
 *
 * 移動平均を使用して安定したFPS値を計算します。
 * 指定した期間のフレーム時間を記録し、その平均からFPSを算出します。
 */
class FpsCounter {
 public:
  /**
   * @brief コンストラクタ
   * @param sample_count 平均計算に使用するサンプル数（デフォルト: 60フレーム）
   */
  explicit FpsCounter(size_t sample_count = 60)
      : sample_count_(sample_count), last_time_(SDL_GetTicks()) {}

  /**
   * @brief フレームを記録してFPSを更新
   *
   * 毎フレーム呼び出す必要があります。
   * 前回のupdate()からの経過時間を記録し、移動平均を計算します。
   */
  void update() {
    Uint64 current_time = SDL_GetTicks();
    Uint64 delta_time = current_time - last_time_;
    last_time_ = current_time;

    // フレーム時間を記録
    frame_times_.push_back(delta_time);

    // サンプル数を超えたら古いデータを削除
    if (frame_times_.size() > sample_count_) {
      frame_times_.pop_front();
    }
  }

  /**
   * @brief 現在のFPSを取得
   *
   * 移動平均から計算されたFPS値を返します。
   * サンプルが十分に集まっていない場合は0.0を返します。
   *
   * @return FPS値
   */
  float getFps() const {
    if (frame_times_.empty()) {
      return 0.0f;
    }

    // 平均フレーム時間を計算
    Uint64 total = 0;
    for (Uint64 time : frame_times_) {
      total += time;
    }
    float avg_frame_time = static_cast<float>(total) / frame_times_.size();

    // FPSを計算（ミリ秒なので1000で割る）
    if (avg_frame_time <= 0.0f) {
      return 0.0f;
    }
    return 1000.0f / avg_frame_time;
  }

  /**
   * @brief 平均フレーム時間を取得（ミリ秒）
   * @return 平均フレーム時間
   */
  float getAverageFrameTime() const {
    if (frame_times_.empty()) {
      return 0.0f;
    }

    Uint64 total = 0;
    for (Uint64 time : frame_times_) {
      total += time;
    }
    return static_cast<float>(total) / frame_times_.size();
  }

  /**
   * @brief 最後のフレーム時間を取得（ミリ秒）
   * @return 最後のフレーム時間
   */
  float getLastFrameTime() const {
    if (frame_times_.empty()) {
      return 0.0f;
    }
    return static_cast<float>(frame_times_.back());
  }

  /**
   * @brief リセット
   *
   * 記録されているフレーム時間をすべてクリアします。
   */
  void reset() {
    frame_times_.clear();
    last_time_ = SDL_GetTicks();
  }

 private:
  size_t sample_count_;          // サンプル数
  std::deque<Uint64> frame_times_;  // フレーム時間の履歴
  Uint64 last_time_;             // 前回のupdate()時刻
};

}  // namespace MyGame::Utilities
