#pragma once

#include <SDL3/SDL.h>
#include <span>
#include <vector>
#include "../core/synthesizer.h"
#include "../types/note.h"
#include "../utilities/fixed_note_sequence.h"

namespace MySound {

/**
 * @brief シーケンサー
 *
 * 音符のシーケンスを再生します。
 * SDL_AddTimerNSを使用した高精度タイマーで音符の進行を制御します。
 */
class Sequencer {
 public:
  /**
   * @brief コンストラクタ
   * @param synthesizer 使用するシンセサイザー
   * @param bpm BPM（Beats Per Minute）
   */
  Sequencer(SimpleSynthesizer* synthesizer, float bpm = 120.0f);

  /**
   * @brief デストラクタ
   */
  ~Sequencer();

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
   * @brief シーケンサーの更新間隔を取得（ナノ秒）
   * @return 更新間隔（ナノ秒）
   */
  Uint64 getUpdateIntervalNS() const { return update_interval_ns_; }

  /**
   * @brief シーケンサーの更新間隔を取得（ミリ秒）
   * @return 更新間隔（ミリ秒）
   */
  Uint32 getUpdateInterval() const {
    return static_cast<Uint32>(update_interval_ns_ / 1000000);  // ns to ms
  }

  /**
   * @brief BPMを設定
   * @param bpm BPM
   */
  void setBPM(float bpm) { bpm_ = bpm; }

  /**
   * @brief BPMを取得
   * @return BPM
   */
  float getBPM() const { return bpm_; }

  /**
   * @brief シーケンサーのボリュームを設定
   * @param volume ボリューム（0.0〜1.0）
   */
  void setVolume(float volume);

  /**
   * @brief シーケンサーのボリュームを取得
   * @return ボリューム（0.0〜1.0）
   */
  float getVolume() const { return volume_; }

  /**
   * @brief ループ再生を設定
   * @param enabled ループを有効にするか
   * @param count ループ回数（-1で無限ループ、0以上で指定回数、0で1回のみ再生）
   */
  void setLoop(bool enabled, int count = -1);

  /**
   * @brief ループが有効かどうか
   * @return ループが有効ならtrue
   */
  bool isLoopEnabled() const { return loop_enabled_; }

  /**
   * @brief 現在のループ回数を取得
   * @return 現在のループ回数（0始まり）
   */
  int getCurrentLoop() const { return current_loop_; }

  /**
   * @brief シーケンスをクリア
   */
  void clear();

  /**
   * @brief 音符を追加
   * @param note 音階
   * @param octave オクターブ
   * @param note_division 音符の分数（4 = 4分音符）
   * @param dotted 付点音符かどうか
   * @param wave_type 音色（オシレーター）
   * @param volume ボリューム（0.0〜1.0）
   */
  void addNote(Note note, int octave, int note_division, bool dotted = false,
               WaveType wave_type = WaveType::Sine, float volume = 1.0f);

  /**
   * @brief 休符を追加
   * @param note_division 音符の分数（4 = 4分音符）
   * @param dotted 付点音符かどうか
   */
  void addRest(int note_division, bool dotted = false);

  /**
   * @brief MMLから生成された音符シーケンスを設定（std::vector版）
   * @param notes 音符データのリスト
   */
  void setSequence(std::vector<NoteData>&& notes);

  /**
   * @brief MMLから生成された音符シーケンスを設定（std::vector版）
   * @param notes 音符データのリスト
   */
  void setSequence(const std::vector<NoteData>& notes);

  /**
   * @brief MMLから生成された音符シーケンスを設定（FixedNoteSequence版）
   * @param notes 固定長音符シーケンス（constexpr対応）
   */
  void setSequence(const FixedNoteSequence& notes);

  /**
   * @brief 任意のコンテナから音符シーケンスを設定（std::span版）
   * @param notes 音符データのスパン（配列、vector、FixedNoteSequenceなど）
   */
  void setSequence(std::span<const NoteData> notes);

  /**
   * @brief シーケンスを再生開始
   */
  void play();

  /**
   * @brief シーケンスを停止
   */
  void stop();

  /**
   * @brief 再生中かどうか
   * @return 再生中ならtrue
   */
  bool isPlaying() const { return is_playing_; }

  /**
   * @brief 更新（メインループから毎フレーム呼び出す）
   *
   * 注: 実際の音符進行処理は内部タイマーで行われるため、
   * このメソッドは状態チェック程度の軽量処理のみ行う
   */
  void update();

 private:
  /**
   * @brief タイマーコールバック（静的関数、SDL_AddTimerNS用）
   * @param userdata Sequencerインスタンスへのポインタ
   * @param timerID タイマーID
   * @param interval タイマー間隔（ナノ秒）
   * @return 次のタイマー間隔（0で停止）
   */
  static Uint64 SDLCALL timerCallback(void* userdata, SDL_TimerID timerID, Uint64 interval);

  /**
   * @brief タイマーを開始
   */
  void startTimer();

  /**
   * @brief タイマーを停止
   */
  void stopTimer();

  /**
   * @brief 内部更新処理（タイマーから呼ばれる）
   */
  void internalUpdate();

  /**
   * @brief シーケンス終了時の処理
   */
  void handleSequenceEnd();

  /**
   * @brief 現在の音符を再生
   */
  void playCurrentNote();

  SimpleSynthesizer* synthesizer_;         // シンセサイザー
  float bpm_;                              // BPM
  float volume_;                           // シーケンサーのボリューム（0.0〜1.0）
  std::vector<NoteData> sequence_;         // 音符のシーケンス
  size_t current_note_index_;              // 現在の音符インデックス
  bool is_playing_;                        // 再生中フラグ
  float sequence_time_;                    // シーケンス内の経過時間
  Uint64 last_update_time_;                // 前回の更新時刻
  bool loop_enabled_;                      // ループ有効フラグ
  int loop_count_;                         // ループ回数（-1=無限、0以上=指定回数）
  int current_loop_;                       // 現在のループ回数
  SDL_TimerID timer_id_;                   // タイマーID
  Uint64 update_interval_ns_;              // 更新間隔（ナノ秒）
};

}  // namespace MySound
