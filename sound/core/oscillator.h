#pragma once

#include <SDL3/SDL.h>
#include "../types/wave_type.h"
#include "../sound_constants.h"

namespace MySound {

/**
 * @brief オシレーター（発振器）
 *
 * 指定された周波数で波形を生成します。
 * サイン波、矩形波、ノコギリ波、ホワイトノイズに対応しています。
 */
class Oscillator {
 public:
  /**
   * @brief コンストラクタ
   * @param wave_type 波形の種類
   * @param frequency 周波数（Hz）
   */
  explicit Oscillator(WaveType wave_type = WaveType::Sine,
                      float frequency = DEFAULT_FREQUENCY);

  /**
   * @brief 波形の種類を設定
   * @param wave_type 波形の種類
   */
  void setWaveType(WaveType wave_type);

  /**
   * @brief 周波数を設定
   * @param frequency 周波数（Hz）
   */
  void setFrequency(float frequency);

  /**
   * @brief 波形の種類を取得
   * @return 波形の種類
   */
  WaveType getWaveType() const;

  /**
   * @brief 周波数を取得
   * @return 周波数（Hz）
   */
  float getFrequency() const;

  /**
   * @brief ノイズジェネレーターのシードを設定
   * @param seed シード値
   */
  void setNoiseSeed(Uint32 seed);

  /**
   * @brief 位相から波形の値を生成
   * @param phase 位相（0.0〜1.0）
   * @return 波形の値（-1.0〜1.0）
   */
  float generate(float phase) const;

 private:
  /**
   * @brief ホワイトノイズを生成（LCG方式）
   * @return ノイズ値（-1.0〜1.0）
   */
  float generateNoise() const;

  WaveType wave_type_;           // 波形の種類
  float frequency_;              // 周波数（Hz）
  mutable Uint32 noise_state_;   // ノイズジェネレーター状態（LCG用）
};

}  // namespace MySound
