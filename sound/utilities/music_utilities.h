#pragma once

#include "../types/note.h"
#include "../sound_constants.h"

namespace MySound {

/**
 * @brief 音楽関連のユーティリティ関数（constexpr対応）
 */
class MusicUtil {
 public:
  /**
   * @brief 音階とオクターブから周波数を計算
   * @param note 音階
   * @param octave オクターブ
   * @return 周波数（Hz）
   */
  static constexpr float noteToFrequency(Note note, int octave) {
    // A4（ラ）を基準（440Hz）とする
    int semitones_from_a4 = (octave - 4) * 12 + (static_cast<int>(note) - 9);
    return 440.0f * constexpr_pow(2.0f, semitones_from_a4 / 12.0f);
  }

  /**
   * @brief BPMと音符の長さから演奏時間を計算
   * @param bpm BPM（Beats Per Minute）
   * @param note_division 音符の分割数（4なら4分音符、8なら8分音符）
   * @param dotted 付点音符かどうか
   * @return 演奏時間（秒）
   */
  static constexpr float noteDuration(float bpm, int note_division, bool dotted = false) {
    // 4分音符の長さ = 60 / BPM（秒）
    float quarter_note = 60.0f / bpm;

    // 指定された音符の長さ
    float duration = quarter_note * 4.0f / static_cast<float>(note_division);

    // 付点音符の場合は1.5倍
    if (dotted) {
      duration *= 1.5f;
    }

    return duration;
  }

 private:
  /**
   * @brief constexpr対応のべき乗関数（簡易実装）
   * @param base 底
   * @param exp 指数
   * @return base^exp
   */
  static constexpr float constexpr_pow(float base, float exp) {
    // 2^xの近似: exp(x * ln(2))
    float x = exp * 0.693147f;  // ln(2) ≈ 0.693147

    // e^xのテイラー展開
    float result = 1.0f;
    float term = 1.0f;
    for (int i = 1; i < 20; ++i) {
      term *= x / i;
      result += term;
    }
    return result;
  }
};

}  // namespace MySound
