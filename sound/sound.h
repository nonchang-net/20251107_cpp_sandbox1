#pragma once

/**
 * @file sound.h
 * @brief MySoundライブラリの統合ヘッダ
 *
 * このファイルをインクルードするだけで、全てのサウンド機能が利用可能になります。
 *
 * Phase 1: 基本的なシンセサイザー機能
 * - Oscillator: 波形生成（Sine, Square, Sawtooth, Noise）
 * - Envelope: ADSR エンベロープ
 * - SimpleSynthesizer: 基本的なシンセサイザー
 *
 * 使用例:
 * @code
 * #include "sound/sound.h"
 *
 * using namespace MySound;
 *
 * // シンセサイザーを作成
 * auto synth = std::make_unique<SimpleSynthesizer>(44100);
 *
 * // エンベロープを設定
 * synth->getEnvelope().setADSR(0.01f, 0.1f, 0.7f, 0.2f);
 *
 * // 音を鳴らす
 * synth->noteOn(440.0f, 1.0f);  // A4（ラ）を1秒間
 *
 * // 毎フレーム更新
 * synth->update();
 * @endcode
 */

// 定数
#include "sound_constants.h"

// 基本型
#include "types/wave_type.h"
#include "types/note.h"

// ユーティリティ
#include "utilities/music_utilities.h"
#include "utilities/fixed_note_sequence.h"

// コアクラス
#include "core/oscillator.h"
#include "core/envelope.h"
#include "core/synthesizer.h"

// MML
#include "mml/mml_parser.h"

// シーケンサー
#include "sequencer/sequencer.h"
#include "sequencer/multi_track_sequencer.h"
#include "sequencer/bgm_manager.h"

// エフェクト
#include "effect/audio_effect.h"
#include "effect/biquad_filter.h"
#include "effect/volume_modulation.h"

// ミキサー
#include "mixer/audio_mixer.h"
