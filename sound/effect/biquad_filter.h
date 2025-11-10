#pragma once

#include <SDL3/SDL.h>

namespace MySound {

/**
 * @brief Biquadフィルターのタイプ
 *
 * Web Audio API BiquadFilterNodeのtypeに準拠
 */
enum class BiquadFilterType {
  Lowpass,   // ローパスフィルター（低周波数を通過）
  Highpass,  // ハイパスフィルター（高周波数を通過）
  Bandpass,  // バンドパスフィルター（特定周波数帯域を通過）
  Lowshelf,  // ローシェルフフィルター（低周波数帯域をブースト/カット）
  Highshelf, // ハイシェルフフィルター（高周波数帯域をブースト/カット）
  Peaking,   // ピーキングフィルター（特定周波数帯域をブースト/カット）
  Notch,     // ノッチフィルター（特定周波数を除去）
  Allpass    // オールパスフィルター（位相特性のみ変更）
};

/**
 * @brief Biquadフィルター
 *
 * Web Audio APIのBiquadFilterNodeを参考にした2次IIRフィルター。
 * オシレーターやシンセサイザーの出力に対してフィルタリングを行います。
 *
 * 使用例:
 * @code
 * auto filter = std::make_unique<BiquadFilter>(44100);
 * filter->setType(BiquadFilterType::Lowpass);
 * filter->setFrequency(1000.0f);  // 1kHz
 * filter->setQ(1.0f);
 *
 * // サンプル処理
 * float output = filter->process(input_sample);
 * @endcode
 *
 * 参考: Robert Bristow-Johnson "Cookbook formulae for audio EQ biquad filter coefficients"
 */
class BiquadFilter {
 public:
  /**
   * @brief コンストラクタ
   * @param sample_rate サンプリングレート（Hz）
   */
  explicit BiquadFilter(int sample_rate);

  /**
   * @brief フィルタータイプを設定
   * @param type フィルタータイプ
   */
  void setType(BiquadFilterType type);

  /**
   * @brief カットオフ周波数を設定
   * @param frequency 周波数（Hz）、範囲: 10Hz〜sample_rate/2
   */
  void setFrequency(float frequency);

  /**
   * @brief クオリティファクター（Q値）を設定
   * @param q Q値、範囲: 0.0001〜1000.0（デフォルト: 1.0）
   */
  void setQ(float q);

  /**
   * @brief ゲインを設定（Peaking/Lowshelf/Highshelfで使用）
   * @param gain ゲイン（dB）、範囲: -40.0〜40.0（デフォルト: 0.0）
   */
  void setGain(float gain);

  /**
   * @brief デチューンを設定（周波数の微調整）
   * @param detune デチューン（セント）、範囲: -1200.0〜1200.0（デフォルト: 0.0）
   */
  void setDetune(float detune);

  /**
   * @brief フィルタータイプを取得
   * @return フィルタータイプ
   */
  BiquadFilterType getType() const { return type_; }

  /**
   * @brief カットオフ周波数を取得
   * @return 周波数（Hz）
   */
  float getFrequency() const { return frequency_; }

  /**
   * @brief Q値を取得
   * @return Q値
   */
  float getQ() const { return q_; }

  /**
   * @brief ゲインを取得
   * @return ゲイン（dB）
   */
  float getGain() const { return gain_; }

  /**
   * @brief デチューンを取得
   * @return デチューン（セント）
   */
  float getDetune() const { return detune_; }

  /**
   * @brief サンプルを処理（フィルタリング）
   * @param input 入力サンプル
   * @return フィルタリング後のサンプル
   */
  float process(float input);

  /**
   * @brief フィルター状態をリセット
   */
  void reset();

 private:
  /**
   * @brief フィルター係数を再計算
   */
  void updateCoefficients();

  /**
   * @brief デチューンを適用した周波数を計算
   * @return デチューン適用後の周波数（Hz）
   */
  float getDetunedFrequency() const;

  int sample_rate_;                  // サンプリングレート
  BiquadFilterType type_;            // フィルタータイプ
  float frequency_;                  // カットオフ周波数（Hz）
  float q_;                          // クオリティファクター
  float gain_;                       // ゲイン（dB）
  float detune_;                     // デチューン（セント）

  // Biquadフィルター係数（Direct Form I）
  float b0_, b1_, b2_;  // フィードフォワード係数
  float a0_, a1_, a2_;  // フィードバック係数

  // フィルター状態（Direct Form I）
  float x1_, x2_;  // 入力の過去2サンプル
  float y1_, y2_;  // 出力の過去2サンプル
};

}  // namespace MySound
