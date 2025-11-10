#pragma once

#include <SDL3/SDL.h>

namespace MySound {

/**
 * @brief サウンドライブラリ全体で使用する定数
 */

// サンプリングレート
constexpr int DEFAULT_SAMPLE_RATE = 44100;

// シーケンサーのデフォルト更新間隔
constexpr Uint64 DEFAULT_SEQUENCER_UPDATE_INTERVAL_NS = 15000000;  // 15ms（32分音符精度）
constexpr Uint32 DEFAULT_SEQUENCER_UPDATE_INTERVAL_MS = 15;         // 15ms

// BPM
constexpr float DEFAULT_BPM = 120.0f;

// ボリューム
constexpr float DEFAULT_VOLUME = 1.0f;
constexpr float MIN_VOLUME = 0.0f;
constexpr float MAX_VOLUME = 1.0f;

// ADSRエンベロープのデフォルト値
constexpr float DEFAULT_ATTACK_TIME = 0.01f;   // 10ms
constexpr float DEFAULT_DECAY_TIME = 0.1f;     // 100ms
constexpr float DEFAULT_SUSTAIN_LEVEL = 0.7f;  // 70%
constexpr float DEFAULT_RELEASE_TIME = 0.2f;   // 200ms

// オシレーターのデフォルト値
constexpr float DEFAULT_FREQUENCY = 440.0f;  // A4

// ノイズジェネレーターのデフォルトseed
constexpr Uint32 DEFAULT_NOISE_SEED = 0x12345678;

// LCGパラメータ（Numerical Recipes推奨）
constexpr Uint32 LCG_MULTIPLIER = 1664525u;
constexpr Uint32 LCG_INCREMENT = 1013904223u;

// MMLパーサーのデフォルト値
constexpr int DEFAULT_OCTAVE = 4;
constexpr int DEFAULT_NOTE_LENGTH = 4;  // 4分音符
constexpr float DEFAULT_NOTE_VOLUME = 1.0f;

// 音符の最大数（FixedNoteSequence用）
constexpr size_t MAX_NOTE_SEQUENCE_SIZE = 512;

}  // namespace MySound
