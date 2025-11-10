#pragma once

#include <SDL3/SDL.h>
#include "../sound_constants.h"

namespace MySound {

/**
 * @brief ADSRエンベロープ
 *
 * Attack, Decay, Sustain, Releaseの4つのパラメータで
 * 音の時間的な変化を制御します。
 */
class Envelope {
 public:
  /**
   * @brief エンベロープの状態
   */
  enum class State {
    Idle,     // アイドル（音が鳴っていない）
    Attack,   // アタック（音量が上昇）
    Decay,    // ディケイ（サステインレベルまで減衰）
    Sustain,  // サステイン（一定の音量を保持）
    Release   // リリース（音量が減衰）
  };

  /**
   * @brief コンストラクタ
   */
  Envelope();

  /**
   * @brief ADSRパラメータを一括設定
   * @param attack_time アタック時間（秒）
   * @param decay_time ディケイ時間（秒）
   * @param sustain_level サステインレベル（0.0〜1.0）
   * @param release_time リリース時間（秒）
   */
  void setADSR(float attack_time, float decay_time, float sustain_level, float release_time);

  /**
   * @brief アタック時間を設定
   * @param time アタック時間（秒）
   */
  void setAttackTime(float time);

  /**
   * @brief ディケイ時間を設定
   * @param time ディケイ時間（秒）
   */
  void setDecayTime(float time);

  /**
   * @brief サステインレベルを設定
   * @param level サステインレベル（0.0〜1.0）
   */
  void setSustainLevel(float level);

  /**
   * @brief リリース時間を設定
   * @param time リリース時間（秒）
   */
  void setReleaseTime(float time);

  /**
   * @brief エンベロープを開始（ノートオン）
   */
  void noteOn();

  /**
   * @brief エンベロープを終了（ノートオフ）
   */
  void noteOff();

  /**
   * @brief エンベロープの値を計算
   * @param sample_rate サンプリングレート
   * @return エンベロープ値（0.0〜1.0）
   */
  float process(int sample_rate);

  /**
   * @brief 現在の状態を取得
   * @return エンベロープの状態
   */
  State getState() const;

 private:
  float attack_time_;     // アタック時間（秒）
  float decay_time_;      // ディケイ時間（秒）
  float sustain_level_;   // サステインレベル（0.0〜1.0）
  float release_time_;    // リリース時間（秒）

  State state_;           // 現在の状態
  float current_level_;   // 現在のレベル
  float release_level_;   // リリース開始時のレベル
};

}  // namespace MySound
