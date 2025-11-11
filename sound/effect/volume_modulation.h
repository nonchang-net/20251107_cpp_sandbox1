#pragma once

#include <memory>
#include "audio_effect.h"
#include "../core/oscillator.h"

namespace MySound {

/**
 * @brief ボリュームモジュレーション（トレモロ効果）
 *
 * LFO（Low Frequency Oscillator）を使用してボリュームを周期的に変調します。
 * 古典的なトレモロエフェクトを実現します。
 *
 * 使用例:
 * @code
 * auto tremolo = std::make_unique<VolumeModulation>(44100);
 * tremolo->setRate(5.0f);              // 5Hz（1秒間に5回）
 * tremolo->setDepth(0.5f);             // 50%の深さ
 * tremolo->setWaveType(WaveType::Sine); // サイン波でモジュレーション
 *
 * float output = tremolo->process(input_sample);
 * @endcode
 */
class VolumeModulation : public AudioEffect {
 public:
  /**
   * @brief コンストラクタ
   * @param sample_rate サンプリングレート（Hz）
   */
  explicit VolumeModulation(int sample_rate);

  /**
   * @brief モジュレーション周波数（レート）を設定
   * @param rate 周波数（Hz）、範囲: 0.1Hz〜20.0Hz（推奨）
   */
  void setRate(float rate);

  /**
   * @brief モジュレーションの深さ（効果量）を設定
   * @param depth 深さ（0.0〜1.0）
   *              0.0 = エフェクトなし
   *              1.0 = 最大効果（ボリュームが0〜100%で変化）
   */
  void setDepth(float depth);

  /**
   * @brief LFO波形タイプを設定
   * @param wave_type 波形タイプ
   */
  void setWaveType(WaveType wave_type);

  /**
   * @brief モジュレーション周波数を取得
   * @return 周波数（Hz）
   */
  float getRate() const { return rate_; }

  /**
   * @brief モジュレーションの深さを取得
   * @return 深さ（0.0〜1.0）
   */
  float getDepth() const { return depth_; }

  /**
   * @brief LFO波形タイプを取得
   * @return 波形タイプ
   */
  WaveType getWaveType() const;

  /**
   * @brief サンプルを処理（AudioEffectインターフェース実装）
   * @param input 入力サンプル
   * @return ボリュームモジュレーション適用後のサンプル
   */
  float process(float input) override;

  /**
   * @brief エフェクト状態をリセット（AudioEffectインターフェース実装）
   */
  void reset() override;

 private:
  int sample_rate_;                      // サンプリングレート
  float rate_;                           // モジュレーション周波数（Hz）
  float depth_;                          // 効果量（0.0〜1.0）
  std::unique_ptr<Oscillator> lfo_;      // LFO（Low Frequency Oscillator）
  Uint64 current_sample_;                // 現在のサンプル位置
};

}  // namespace MySound
