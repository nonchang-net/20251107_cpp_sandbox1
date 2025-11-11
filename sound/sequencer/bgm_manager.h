#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "multi_track_sequencer.h"

namespace MySound {

/**
 * @brief BGMマネージャー
 *
 * 複数の楽曲（MultiTrackSequencer）を管理し、切り替える。
 * クロスフェード機能により、BGM間をスムーズに遷移できる。
 */
class BGMManager {
 public:
  /**
   * @brief コンストラクタ
   * @param sample_rate サンプリングレート
   */
  explicit BGMManager(int sample_rate = 44100);

  /**
   * @brief BGMを登録
   * @param id BGM ID
   * @param bgm マルチトラックシーケンサー（ストリームなしモードで作成すること）
   */
  void registerBGM(const std::string& id, std::unique_ptr<MultiTrackSequencer> bgm);

  /**
   * @brief BGMを取得
   * @param id BGM ID
   * @return マルチトラックシーケンサーへのポインタ（存在しない場合はnullptr）
   */
  MultiTrackSequencer* getBGM(const std::string& id);

  /**
   * @brief BGMを再生
   * @param id BGM ID
   * @return 成功したらtrue
   */
  bool play(const std::string& id);

  /**
   * @brief BGMをクロスフェードで再生
   * @param id BGM ID
   * @param fade_duration フェード時間（秒）
   * @return 成功したらtrue
   */
  bool playWithCrossfade(const std::string& id, float fade_duration = 2.0f);

  /**
   * @brief 現在のBGMを停止
   */
  void stop();

  /**
   * @brief 現在のBGMを一時停止
   */
  void pause();

  /**
   * @brief 現在のBGMを再開
   */
  void resume();

  /**
   * @brief 全BGMのマスターボリュームを設定
   * @param volume ボリューム（0.0〜1.0）
   */
  void setMasterVolume(float volume);

  /**
   * @brief マスターボリュームを取得
   * @return ボリューム（0.0〜1.0）
   */
  float getMasterVolume() const { return master_volume_; }

  /**
   * @brief 現在再生中のBGM IDを取得
   * @return BGM ID（再生中でない場合は空文字列）
   */
  const std::string& getCurrentBGMId() const { return current_bgm_id_; }

  /**
   * @brief BGMが再生中かどうか
   * @return 再生中ならtrue
   */
  bool isPlaying() const;

  /**
   * @brief 更新（メインループから毎フレーム呼び出す）
   */
  void update();

 private:
  /**
   * @brief オーディオコールバック（静的メソッド）
   */
  static void SDLCALL audioCallback(void* userdata, SDL_AudioStream* stream,
                                    int additional_amount, int total_amount);

  /**
   * @brief サンプルをミックス
   */
  void mixSamples(float* output, int num_samples);
  /**
   * @brief フェード状態
   */
  struct FadeState {
    MultiTrackSequencer* bgm = nullptr;  // フェード中のBGM
    std::string bgm_id;                   // BGM ID
    float current_volume = 0.0f;          // 現在のボリューム
    float target_volume = 0.0f;           // 目標ボリューム
    float fade_duration = 0.0f;           // フェード時間（秒）
    float elapsed_time = 0.0f;            // 経過時間（秒）
    bool is_fading = false;               // フェード中フラグ
  };

  /**
   * @brief フェード処理を更新
   * @param state フェード状態
   * @param delta_time デルタタイム（秒）
   */
  void updateFade(FadeState& state, float delta_time);

  std::unordered_map<std::string, std::unique_ptr<MultiTrackSequencer>> bgm_map_;
  SDL_AudioStream* stream_;  // オーディオストリーム

  std::string current_bgm_id_;
  float master_volume_ = 1.0f;
  int sample_rate_;

  // クロスフェード管理
  FadeState fade_in_;   // フェードイン中のBGM
  FadeState fade_out_;  // フェードアウト中のBGM
  Uint64 last_update_time_ = 0;  // 前回の更新時刻（ミリ秒）
};

}  // namespace MySound
