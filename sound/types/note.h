#pragma once

#include <cmath>
#include "wave_type.h"

namespace MySound {

/**
 * @brief 音階（12平均律）
 */
enum class Note {
  C = 0,   // ド
  Cs = 1,  // ド#
  D = 2,   // レ
  Ds = 3,  // レ#
  E = 4,   // ミ
  F = 5,   // ファ
  Fs = 6,  // ファ#
  G = 7,   // ソ
  Gs = 8,  // ソ#
  A = 9,   // ラ
  As = 10, // ラ#
  B = 11   // シ
};

/**
 * @brief 音符データ
 *
 * MMLパーサーやシーケンサーで使用する音符情報を保持します。
 * constexpr対応のため、全てのメンバーがconstexprで初期化可能です。
 */
struct NoteData {
  Note note;              // 音階
  int octave;             // オクターブ（0〜8）
  float duration;         // 音符の長さ（秒）
  bool is_rest;           // 休符かどうか
  WaveType wave_type;     // 波形の種類
  float volume;           // ボリューム（0.0〜1.0）

  /**
   * @brief コンストラクタ
   * @param n 音階
   * @param oct オクターブ
   * @param dur 音符の長さ（秒）
   * @param rest 休符かどうか
   * @param wave 波形の種類
   * @param vol ボリューム（0.0〜1.0）
   */
  constexpr NoteData(Note n = Note::C, int oct = 4, float dur = 0.5f,
                     bool rest = false, WaveType wave = WaveType::Sine,
                     float vol = 1.0f)
      : note(n), octave(oct), duration(dur), is_rest(rest),
        wave_type(wave), volume(vol) {}

  /**
   * @brief 音符の周波数を計算
   * @return 周波数（Hz）
   *
   * A4（ラ）を440Hzとする12平均律で計算します。
   * 周波数 = 440 × 2^((オクターブ-4)*12 + 音階-9) / 12)
   */
  constexpr float getFrequency() const {
    if (is_rest) return 0.0f;

    // A4（ラ）を基準（440Hz）とする
    // A4はオクターブ4、音階9（Note::A）
    int semitones_from_a4 = (octave - 4) * 12 + (static_cast<int>(note) - 9);

    // 2^(n/12)を計算
    // C++20のstd::powはconstexprだが、SDL3環境では使えない可能性があるため
    // 簡易実装（コンパイル時評価用）
    return 440.0f * constexpr_pow(2.0f, semitones_from_a4 / 12.0f);
  }

private:
  /**
   * @brief constexpr対応のべき乗関数（簡易実装）
   * @param base 底
   * @param exp 指数
   * @return base^exp
   *
   * テイラー展開による近似（精度は実用レベル）
   */
  static constexpr float constexpr_pow(float base, float exp) {
    // 2^xの近似: exp(x * ln(2))
    // ln(2) ≈ 0.693147
    float x = exp * 0.693147f;

    // e^xのテイラー展開: 1 + x + x^2/2! + x^3/3! + ...
    float result = 1.0f;
    float term = 1.0f;
    for (int i = 1; i < 20; ++i) {  // 20項で十分な精度
      term *= x / i;
      result += term;
    }
    return result;
  }
};

}  // namespace MySound
