#pragma once

#include <memory>
#include <vector>
#include "sequencer.h"
#include "../core/synthesizer.h"
#include "../mixer/audio_mixer.h"
#include "../effect/audio_effect.h"
#include "../types/note.h"
#include "../utilities/fixed_note_sequence.h"

namespace MySound {

/**
 * @brief マルチトラックシーケンサー
 *
 * 1つの楽曲を複数トラック（パート）で構成する。
 * 各トラックは独立したシンセサイザーとシーケンサーを持つ。
 */
class MultiTrackSequencer {
 public:
  /**
   * @brief コンストラクタ
   * @param track_count トラック数
   * @param sample_rate サンプリングレート
   * @param bpm BPM
   * @param enable_stream オーディオストリームを有効にするか（falseの場合はサンプル生成のみ）
   */
  explicit MultiTrackSequencer(size_t track_count, int sample_rate = 44100, float bpm = 120.0f, bool enable_stream = true);

  /**
   * @brief トラック数を取得
   * @return トラック数
   */
  size_t getTrackCount() const { return track_count_; }

  /**
   * @brief 指定トラックのシンセサイザーを取得
   * @param track_index トラックインデックス
   * @return シンセサイザーへのポインタ（範囲外の場合はnullptr）
   */
  SimpleSynthesizer* getSynthesizer(size_t track_index);

  /**
   * @brief 指定トラックのシーケンサーを取得
   * @param track_index トラックインデックス
   * @return シーケンサーへのポインタ（範囲外の場合はnullptr）
   */
  Sequencer* getSequencer(size_t track_index);

  /**
   * @brief 指定トラックにシーケンスを設定
   * @param track_index トラックインデックス
   * @param notes 音符シーケンス
   */
  void setTrackSequence(size_t track_index, const FixedNoteSequence& notes);

  /**
   * @brief 指定トラックにシーケンスを設定（std::vector版）
   * @param track_index トラックインデックス
   * @param notes 音符シーケンス
   */
  void setTrackSequence(size_t track_index, const std::vector<NoteData>& notes);

  /**
   * @brief マスターボリュームを設定（全トラックに適用）
   * @param volume ボリューム（0.0〜1.0）
   */
  void setMasterVolume(float volume);

  /**
   * @brief マスターボリュームを取得
   * @return ボリューム（0.0〜1.0）
   */
  float getMasterVolume() const { return master_volume_; }

  /**
   * @brief ループ設定（全トラックに適用）
   * @param enabled ループを有効にするか
   * @param count ループ回数（-1で無限ループ）
   */
  void setLoop(bool enabled, int count = -1);

  /**
   * @brief シーケンサーの更新間隔を設定（ナノ秒）
   * @param interval_ns 更新間隔（ナノ秒）、小さいほど精度が高い
   */
  void setUpdateIntervalNS(Uint64 interval_ns);

  /**
   * @brief シーケンサーの更新間隔を設定（ミリ秒、利便性のため）
   * @param interval_ms 更新間隔（ミリ秒）
   */
  void setUpdateInterval(Uint32 interval_ms);

  /**
   * @brief 全トラックを再生開始
   */
  void play();

  /**
   * @brief 全トラックを停止
   */
  void stop();

  /**
   * @brief 全トラックを一時停止
   */
  void pause();

  /**
   * @brief 全トラックを再開
   */
  void resume();

  /**
   * @brief 再生中かどうか
   * @return 少なくとも1トラックが再生中ならtrue
   */
  bool isPlaying() const;

  /**
   * @brief 一時停止中かどうか
   * @return 一時停止中ならtrue
   */
  bool isPaused() const { return is_paused_; }

  /**
   * @brief 更新（メインループから毎フレーム呼び出す）
   */
  void update();

  /**
   * @brief ミキサーにエフェクトを追加
   *
   * 全トラックのサブミックス後に適用されるマスターエフェクトを追加します。
   * エフェクトの所有権はミキサーに移動します。
   * 追加したエフェクトは追加順に直列に処理されます。
   *
   * @param effect 追加するエフェクト（所有権を移動）
   *
   * 使用例:
   * @code
   * // マスターリバーブを追加
   * auto reverb = std::make_unique<ReverbEffect>(44100);
   * multi_track->addMasterEffect(std::move(reverb));
   *
   * // マスターコンプレッサーを追加
   * auto compressor = std::make_unique<Compressor>(44100);
   * multi_track->addMasterEffect(std::move(compressor));
   * @endcode
   */
  void addMasterEffect(std::unique_ptr<AudioEffect> effect);

  /**
   * @brief 全てのマスターエフェクトを削除
   */
  void clearMasterEffects();

  /**
   * @brief マスターエフェクト数を取得
   * @return 現在設定されているマスターエフェクトの数
   */
  size_t getMasterEffectCount() const;

  /**
   * @brief ミキサーを取得
   * @return ミキサーへのポインタ
   */
  AudioMixer* getMixer();

  /**
   * @brief サンプルを生成（BGMManager用）
   *
   * この関数は主にBGMManagerから呼ばれることを想定しています。
   * ストリームなしモードで作成されたMultiTrackSequencerの場合、
   * この関数を明示的に呼び出してサンプルを取得します。
   *
   * @param samples 出力バッファ
   * @param num_samples サンプル数
   */
  void generateSamples(float* samples, int num_samples);

  /**
   * @brief サンプリングレートを取得
   * @return サンプリングレート
   */
  int getSampleRate() const;

 private:
  size_t track_count_;
  int sample_rate_;
  float bpm_;
  float master_volume_;
  bool is_paused_;
  std::vector<std::unique_ptr<SimpleSynthesizer>> synthesizers_;
  std::vector<std::unique_ptr<Sequencer>> sequencers_;
  std::unique_ptr<AudioMixer> mixer_;  // オーディオミキサー
};

}  // namespace MySound
