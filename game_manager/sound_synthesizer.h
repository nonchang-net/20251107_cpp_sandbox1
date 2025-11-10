#pragma once

#include <SDL3/SDL.h>
#include <cmath>
#include <memory>
#include <vector>

namespace MyGame {

/**
 * @brief 波形の種類
 */
enum class WaveType {
  Sine,      // サイン波
  Square,    // 矩形波
  Sawtooth   // ノコギリ波
};

/**
 * @brief オシレーター（発振器）
 *
 * 指定された周波数で波形を生成します。
 * サイン波、矩形波、ノコギリ波に対応しています。
 */
class Oscillator {
 public:
  /**
   * @brief コンストラクタ
   * @param wave_type 波形の種類
   * @param frequency 周波数（Hz）
   */
  explicit Oscillator(WaveType wave_type = WaveType::Sine, float frequency = 440.0f)
      : wave_type_(wave_type), frequency_(frequency) {}

  /**
   * @brief 波形の種類を設定
   * @param wave_type 波形の種類
   */
  void setWaveType(WaveType wave_type) { wave_type_ = wave_type; }

  /**
   * @brief 周波数を設定
   * @param frequency 周波数（Hz）
   */
  void setFrequency(float frequency) { frequency_ = frequency; }

  /**
   * @brief 波形の種類を取得
   * @return 波形の種類
   */
  WaveType getWaveType() const { return wave_type_; }

  /**
   * @brief 周波数を取得
   * @return 周波数（Hz）
   */
  float getFrequency() const { return frequency_; }

  /**
   * @brief 位相から波形の値を生成
   * @param phase 位相（0.0〜1.0）
   * @return 波形の値（-1.0〜1.0）
   */
  float generate(float phase) const {
    switch (wave_type_) {
      case WaveType::Sine:
        // サイン波: sin(2π * phase)
        return SDL_sinf(2.0f * SDL_PI_F * phase);

      case WaveType::Square:
        // 矩形波: phase < 0.5 なら 1.0、それ以外は -1.0
        return (phase < 0.5f) ? 1.0f : -1.0f;

      case WaveType::Sawtooth:
        // ノコギリ波: 0→1→0の範囲を-1.0〜1.0にマップ
        return 2.0f * phase - 1.0f;

      default:
        return 0.0f;
    }
  }

 private:
  WaveType wave_type_;  // 波形の種類
  float frequency_;     // 周波数（Hz）
};

/**
 * @brief ADSRエンベロープ
 *
 * Attack, Decay, Sustain, Releaseの4つのパラメータで
 * 音の時間的な変化を制御します。
 */
class Envelope {
 public:
  /**
   * @brief コンストラクタ
   * @param attack_time アタック時間（秒）
   * @param decay_time ディケイ時間（秒）
   * @param sustain_level サステインレベル（0.0〜1.0）
   * @param release_time リリース時間（秒）
   */
  Envelope(float attack_time = 0.01f, float decay_time = 0.1f,
           float sustain_level = 0.7f, float release_time = 0.2f)
      : attack_time_(attack_time), decay_time_(decay_time),
        sustain_level_(sustain_level), release_time_(release_time) {}

  /**
   * @brief アタック時間を設定
   * @param time アタック時間（秒）
   */
  void setAttackTime(float time) { attack_time_ = time; }

  /**
   * @brief ディケイ時間を設定
   * @param time ディケイ時間（秒）
   */
  void setDecayTime(float time) { decay_time_ = time; }

  /**
   * @brief サステインレベルを設定
   * @param level サステインレベル（0.0〜1.0）
   */
  void setSustainLevel(float level) { sustain_level_ = level; }

  /**
   * @brief リリース時間を設定
   * @param time リリース時間（秒）
   */
  void setReleaseTime(float time) { release_time_ = time; }

  /**
   * @brief エンベロープの値を計算
   * @param time ノートオンからの経過時間（秒）
   * @param gate ゲート状態（true = ノートオン、false = ノートオフ）
   * @param note_off_time ノートオフした時刻（秒）、ゲートがfalseの場合のみ使用
   * @return エンベロープの値（0.0〜1.0）
   */
  float process(float time, bool gate, float note_off_time = 0.0f) const {
    if (gate) {
      // ノートオン中
      if (attack_time_ > 0.0f && time < attack_time_) {
        // Attack フェーズ: 0.0 → 1.0
        return time / attack_time_;
      } else if (decay_time_ > 0.0f && time < attack_time_ + decay_time_) {
        // Decay フェーズ: 1.0 → sustain_level
        float decay_progress = (time - attack_time_) / decay_time_;
        return 1.0f - (1.0f - sustain_level_) * decay_progress;
      } else {
        // Sustain フェーズ: sustain_level を維持
        return sustain_level_;
      }
    } else {
      // ノートオフ後（Release フェーズ）
      float release_time = time - note_off_time;
      if (release_time_ > 0.0f && release_time < release_time_) {
        // Release フェーズ: note_off時の値 → 0.0
        float release_start_level = process(note_off_time, true);
        float release_progress = release_time / release_time_;
        return release_start_level * (1.0f - release_progress);
      } else {
        // Release完了
        return 0.0f;
      }
    }
  }

 private:
  float attack_time_;     // アタック時間（秒）
  float decay_time_;      // ディケイ時間（秒）
  float sustain_level_;   // サステインレベル（0.0〜1.0）
  float release_time_;    // リリース時間（秒）
};

/**
 * @brief シンプルなシンセサイザー
 *
 * 1つのオシレーターと1つのエンベロープを組み合わせて音を生成します。
 * SDL3のオーディオストリームを使用してリアルタイム再生します。
 */
class SimpleSynthesizer {
 public:
  /**
   * @brief コンストラクタ
   * @param sample_rate サンプリングレート（Hz）
   */
  explicit SimpleSynthesizer(int sample_rate = 44100)
      : sample_rate_(sample_rate), stream_(nullptr), current_sample_(0),
        is_playing_(false), note_on_time_(0.0f), note_off_time_(0.0f),
        gate_(false), note_duration_(0.0f), debug_first_samples_(false) {
    // デフォルトのオシレーターとエンベロープを設定
    oscillator_ = std::make_unique<Oscillator>(WaveType::Sine, 440.0f);
    envelope_ = std::make_unique<Envelope>(0.01f, 0.1f, 0.7f, 0.2f);

    // オーディオストリームを初期化
    SDL_AudioSpec spec;
    spec.channels = 1;           // モノラル
    spec.format = SDL_AUDIO_F32; // 32ビット浮動小数点
    spec.freq = sample_rate_;

    stream_ = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);

    if (!stream_) {
      SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio device: %s", SDL_GetError());
      return;
    }

    SDL_Log("Audio stream initialized: %p, sample_rate=%d", stream_, sample_rate_);

    // オーディオデバイスを再開（デフォルトは一時停止状態）
    if (!SDL_ResumeAudioStreamDevice(stream_)) {
      SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to resume audio device: %s", SDL_GetError());
    } else {
      SDL_Log("Audio device resumed successfully");
    }
  }

  /**
   * @brief デストラクタ
   */
  ~SimpleSynthesizer() {
    if (stream_) {
      SDL_DestroyAudioStream(stream_);
    }
  }

  /**
   * @brief オシレーターを取得
   * @return オシレーターへの参照
   */
  Oscillator& getOscillator() { return *oscillator_; }

  /**
   * @brief エンベロープを取得
   * @return エンベロープへの参照
   */
  Envelope& getEnvelope() { return *envelope_; }

  /**
   * @brief ノートオン（音を鳴らし始める）
   * @param frequency 周波数（Hz）
   * @param duration 音の長さ（秒）、0.0なら無限（手動でnoteOff()が必要）
   */
  void noteOn(float frequency, float duration = 0.0f) {
    if (!stream_) {
      SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Audio stream not initialized");
      return;
    }

    oscillator_->setFrequency(frequency);
    current_sample_ = 0;  // サンプル位置をリセット
    note_on_time_ = 0.0f;
    note_off_time_ = 0.0f;
    note_duration_ = duration;
    gate_ = true;
    is_playing_ = true;
    debug_first_samples_ = true;  // デバッグログを有効化

    SDL_Log("NoteOn: %.2f Hz, duration: %.2f sec, stream=%p", frequency, duration, stream_);
  }

  /**
   * @brief ノートオフ（音を止め始める）
   */
  void noteOff() {
    if (gate_) {
      note_off_time_ = getCurrentTime();
      gate_ = false;
      SDL_Log("NoteOff at %.2f sec", note_off_time_);
    }
  }

  /**
   * @brief 再生中かどうか
   * @return 再生中ならtrue
   */
  bool isPlaying() const { return is_playing_; }

  /**
   * @brief 更新（メインループから毎フレーム呼び出す）
   */
  void update() {
    if (!stream_ || !is_playing_) return;

    // バッファの状態を確認
    int queued = SDL_GetAudioStreamQueued(stream_);
    const int min_buffer_bytes = sample_rate_ * sizeof(float) / 2;  // 0.5秒分

    if (queued < min_buffer_bytes) {
      // バッファが少なくなったらサンプルを生成
      const int samples_to_generate = 512;
      float samples[samples_to_generate];
      generateSamples(samples, samples_to_generate);

      if (!SDL_PutAudioStreamData(stream_, samples, sizeof(samples))) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to put audio data: %s", SDL_GetError());
      }
    }

    // 自動ノートオフ（duration指定がある場合）
    if (gate_ && note_duration_ > 0.0f) {
      float current_time = getCurrentTime();
      if (current_time >= note_duration_) {
        noteOff();
      }
    }

    // Releaseフェーズ完了チェック（エンベロープが0になったら停止）
    if (!gate_) {
      float current_time = getCurrentTime();
      float envelope_value = envelope_->process(current_time, gate_, note_off_time_);
      if (envelope_value <= 0.0f) {
        is_playing_ = false;
        SDL_Log("Sound finished (envelope reached 0)");
      }
    }
  }

 private:
  /**
   * @brief サンプルを生成
   * @param samples 出力バッファ
   * @param count サンプル数
   */
  void generateSamples(float* samples, int count) {
    for (int i = 0; i < count; ++i) {
      // 現在の時刻（秒）
      float current_time = getCurrentTime();

      // エンベロープの値を計算
      float envelope_value = envelope_->process(current_time, gate_, note_off_time_);

      // 位相を計算（0.0〜1.0）
      float frequency = oscillator_->getFrequency();
      float phase = SDL_fmodf(
          static_cast<float>(current_sample_) * frequency / sample_rate_, 1.0f);

      // 波形を生成
      float wave = oscillator_->generate(phase);

      // エンベロープを適用
      samples[i] = wave * envelope_value;

      // クリッピング防止
      if (samples[i] > 1.0f) samples[i] = 1.0f;
      if (samples[i] < -1.0f) samples[i] = -1.0f;

      // 最初の数サンプルをログ出力（デバッグ用）
      if (debug_first_samples_ && i < 10) {
        SDL_Log("Sample[%d]: time=%.6f, env=%.4f, phase=%.4f, wave=%.4f, output=%.4f",
                i, current_time, envelope_value, phase, wave, samples[i]);
      }

      current_sample_++;
    }

    if (debug_first_samples_) {
      debug_first_samples_ = false;
      SDL_Log("Generated %d samples, frequency=%.2f Hz", count, oscillator_->getFrequency());
    }
  }

  /**
   * @brief 現在の時刻を取得（秒）
   * @return 現在の時刻（秒）
   */
  float getCurrentTime() const {
    return static_cast<float>(current_sample_) / sample_rate_;
  }

  int sample_rate_;                       // サンプリングレート
  SDL_AudioStream* stream_;               // オーディオストリーム
  std::unique_ptr<Oscillator> oscillator_; // オシレーター
  std::unique_ptr<Envelope> envelope_;     // エンベロープ

  Uint64 current_sample_;  // 現在のサンプル位置
  bool is_playing_;        // 再生中フラグ
  float note_on_time_;     // ノートオン時刻（秒）
  float note_off_time_;    // ノートオフ時刻（秒）
  bool gate_;              // ゲート状態（true = ノートオン、false = ノートオフ）
  float note_duration_;    // 音の長さ（秒）、0.0なら無限
  bool debug_first_samples_;  // デバッグ用：最初のサンプルをログ出力
};

/**
 * @brief 音階
 */
enum class Note {
  C = 0,   // ド
  Cs = 1,  // ド#（C#）
  D = 2,   // レ
  Ds = 3,  // レ#（D#）
  E = 4,   // ミ
  F = 5,   // ファ
  Fs = 6,  // ファ#（F#）
  G = 7,   // ソ
  Gs = 8,  // ソ#（G#）
  A = 9,   // ラ
  As = 10, // ラ#（A#）
  B = 11   // シ
};

/**
 * @brief 音楽ユーティリティ
 *
 * 音階から周波数への変換、BPMから音符の長さへの変換などを提供します。
 */
class MusicUtil {
 public:
  /**
   * @brief 音階とオクターブから周波数を計算
   * @param note 音階
   * @param octave オクターブ（A4 = 440Hzが4）
   * @return 周波数（Hz）
   */
  static float noteToFrequency(Note note, int octave) {
    // A4 = 440Hz を基準とする
    // 半音ごとに 2^(1/12) 倍
    const float A4 = 440.0f;
    const int A4_note = static_cast<int>(Note::A);
    const int A4_octave = 4;

    // A4からの半音数を計算
    int semitones_from_A4 = (octave - A4_octave) * 12 +
                            (static_cast<int>(note) - A4_note);

    // 周波数を計算
    return A4 * SDL_powf(2.0f, semitones_from_A4 / 12.0f);
  }

  /**
   * @brief BPMから1拍の長さ（秒）を計算
   * @param bpm BPM（Beats Per Minute）
   * @return 1拍の長さ（秒）
   */
  static float beatDuration(float bpm) {
    return 60.0f / bpm;
  }

  /**
   * @brief BPMと音符の分数から音符の長さ（秒）を計算
   * @param bpm BPM（Beats Per Minute）
   * @param note_division 音符の分数（4 = 4分音符、8 = 8分音符、...）
   * @param dotted 付点音符かどうか（1.5倍になる）
   * @return 音符の長さ（秒）
   */
  static float noteDuration(float bpm, int note_division, bool dotted = false) {
    // 4分音符を1拍とする
    float quarter_note = beatDuration(bpm);
    float duration = quarter_note * (4.0f / note_division);

    if (dotted) {
      duration *= 1.5f;
    }

    return duration;
  }

  /**
   * @brief 全音符の長さを取得
   * @param bpm BPM
   * @return 全音符の長さ（秒）
   */
  static float wholeNote(float bpm) { return noteDuration(bpm, 1); }

  /**
   * @brief 2分音符の長さを取得
   * @param bpm BPM
   * @return 2分音符の長さ（秒）
   */
  static float halfNote(float bpm) { return noteDuration(bpm, 2); }

  /**
   * @brief 4分音符の長さを取得
   * @param bpm BPM
   * @return 4分音符の長さ（秒）
   */
  static float quarterNote(float bpm) { return noteDuration(bpm, 4); }

  /**
   * @brief 8分音符の長さを取得
   * @param bpm BPM
   * @return 8分音符の長さ（秒）
   */
  static float eighthNote(float bpm) { return noteDuration(bpm, 8); }

  /**
   * @brief 16分音符の長さを取得
   * @param bpm BPM
   * @return 16分音符の長さ（秒）
   */
  static float sixteenthNote(float bpm) { return noteDuration(bpm, 16); }

  /**
   * @brief 32分音符の長さを取得
   * @param bpm BPM
   * @return 32分音符の長さ（秒）
   */
  static float thirtySecondNote(float bpm) { return noteDuration(bpm, 32); }

  /**
   * @brief 64分音符の長さを取得
   * @param bpm BPM
   * @return 64分音符の長さ（秒）
   */
  static float sixtyFourthNote(float bpm) { return noteDuration(bpm, 64); }

  /**
   * @brief 128分音符の長さを取得
   * @param bpm BPM
   * @return 128分音符の長さ（秒）
   */
  static float hundredTwentyEighthNote(float bpm) { return noteDuration(bpm, 128); }

  /**
   * @brief 256分音符の長さを取得
   * @param bpm BPM
   * @return 256分音符の長さ（秒）
   */
  static float twoHundredFiftySixthNote(float bpm) { return noteDuration(bpm, 256); }
};

/**
 * @brief 音符データ
 */
struct NoteData {
  Note note;         // 音階
  int octave;        // オクターブ
  float duration;    // 長さ（秒）

  /**
   * @brief コンストラクタ
   * @param n 音階
   * @param oct オクターブ
   * @param dur 長さ（秒）
   */
  NoteData(Note n, int oct, float dur)
      : note(n), octave(oct), duration(dur) {}

  /**
   * @brief 周波数を取得
   * @return 周波数（Hz）
   */
  float getFrequency() const {
    return MusicUtil::noteToFrequency(note, octave);
  }
};

/**
 * @brief シーケンサー
 *
 * 音符のシーケンスを再生します。
 */
class Sequencer {
 public:
  /**
   * @brief コンストラクタ
   * @param synthesizer 使用するシンセサイザー
   * @param bpm BPM（Beats Per Minute）
   */
  Sequencer(SimpleSynthesizer* synthesizer, float bpm = 120.0f)
      : synthesizer_(synthesizer), bpm_(bpm), current_note_index_(0),
        is_playing_(false), sequence_time_(0.0f), last_update_time_(0) {}

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
   * @brief シーケンスをクリア
   */
  void clear() {
    sequence_.clear();
    current_note_index_ = 0;
    sequence_time_ = 0.0f;
  }

  /**
   * @brief 音符を追加
   * @param note 音階
   * @param octave オクターブ
   * @param note_division 音符の分数（4 = 4分音符）
   * @param dotted 付点音符かどうか
   */
  void addNote(Note note, int octave, int note_division, bool dotted = false) {
    float duration = MusicUtil::noteDuration(bpm_, note_division, dotted);
    sequence_.emplace_back(note, octave, duration);
  }

  /**
   * @brief 休符を追加
   * @param note_division 音符の分数（4 = 4分音符）
   * @param dotted 付点音符かどうか
   */
  void addRest(int note_division, bool dotted = false) {
    float duration = MusicUtil::noteDuration(bpm_, note_division, dotted);
    // 周波数0で休符を表現
    sequence_.emplace_back(Note::C, 0, duration);
  }

  /**
   * @brief シーケンスを再生開始
   */
  void play() {
    if (sequence_.empty()) return;

    current_note_index_ = 0;
    sequence_time_ = 0.0f;
    is_playing_ = true;
    last_update_time_ = SDL_GetTicks();

    // 最初の音符を再生
    playCurrentNote();
  }

  /**
   * @brief シーケンスを停止
   */
  void stop() {
    is_playing_ = false;
    synthesizer_->noteOff();
  }

  /**
   * @brief 再生中かどうか
   * @return 再生中ならtrue
   */
  bool isPlaying() const { return is_playing_; }

  /**
   * @brief 更新（メインループから毎フレーム呼び出す）
   */
  void update() {
    if (!is_playing_ || sequence_.empty()) return;

    // 経過時間を計算
    Uint64 current_time = SDL_GetTicks();
    float delta_time = (current_time - last_update_time_) / 1000.0f;
    last_update_time_ = current_time;

    sequence_time_ += delta_time;

    // 現在の音符が終了したか確認
    if (current_note_index_ < sequence_.size()) {
      const NoteData& current_note = sequence_[current_note_index_];
      if (sequence_time_ >= current_note.duration) {
        // 次の音符へ
        sequence_time_ -= current_note.duration;
        current_note_index_++;

        if (current_note_index_ < sequence_.size()) {
          playCurrentNote();
        } else {
          // シーケンス終了
          is_playing_ = false;
        }
      }
    }
  }

 private:
  /**
   * @brief 現在の音符を再生
   */
  void playCurrentNote() {
    if (current_note_index_ >= sequence_.size()) return;

    const NoteData& note_data = sequence_[current_note_index_];

    // 休符の場合は何もしない
    if (note_data.octave == 0) {
      synthesizer_->noteOff();
      return;
    }

    // 音符を再生
    float frequency = note_data.getFrequency();
    synthesizer_->noteOn(frequency, note_data.duration);
  }

  SimpleSynthesizer* synthesizer_;         // シンセサイザー
  float bpm_;                              // BPM
  std::vector<NoteData> sequence_;         // 音符のシーケンス
  size_t current_note_index_;              // 現在の音符インデックス
  bool is_playing_;                        // 再生中フラグ
  float sequence_time_;                    // シーケンス内の経過時間
  Uint64 last_update_time_;                // 前回の更新時刻
};

}  // namespace MyGame
