#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "multi_track_sequencer.h"

namespace MySound {

/**
 * @brief BGMマネージャー
 *
 * 複数の楽曲（MultiTrackSequencer）を管理し、切り替える。
 */
class BGMManager {
 public:
  /**
   * @brief BGMを登録
   * @param id BGM ID
   * @param bgm マルチトラックシーケンサー
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
  std::unordered_map<std::string, std::unique_ptr<MultiTrackSequencer>> bgm_map_;
  std::string current_bgm_id_;
  float master_volume_ = 1.0f;
};

}  // namespace MySound
