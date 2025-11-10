#pragma once

#include <SDL3/SDL.h>
#include <array>
#include <cmath>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

// デバッグログの有効化フラグ（0 = 無効、1 = 有効）
#define SOUND_SYNTHESIZER_DEBUG_LOG 0

// デバッグログ用マクロ
#if SOUND_SYNTHESIZER_DEBUG_LOG
  #define SYNTH_LOG(...) SDL_Log(__VA_ARGS__)
#else
  #define SYNTH_LOG(...) ((void)0)
#endif

namespace MyGame {

/**
 * @brief 波形の種類
 */
enum class WaveType {
  Sine,      // サイン波
  Square,    // 矩形波
  Sawtooth,  // ノコギリ波
  Noise      // ホワイトノイズ（LCG方式）
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
      : wave_type_(wave_type), frequency_(frequency), noise_state_(0x12345678) {}

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

      case WaveType::Noise:
        // ホワイトノイズ: Linear Congruential Generator (LCG)
        // phaseは無視し、独立した疑似乱数列を生成
        return generateNoise();

      default:
        return 0.0f;
    }
  }

 private:
  /**
   * @brief ホワイトノイズを生成（LCG方式）
   * @return ノイズ値（-1.0〜1.0）
   */
  float generateNoise() const {
    // Linear Congruential Generator
    // 参考: Numerical Recipes推奨パラメータ
    // state = (a * state + c) mod 2^32
    noise_state_ = noise_state_ * 1664525u + 1013904223u;

    // uint32_tをint32_tとして解釈し、-1.0〜1.0の範囲に正規化
    // int32_tの範囲は-2^31 ~ 2^31-1なので、2^31で割る
    return static_cast<int32_t>(noise_state_) / 2147483648.0f;
  }

 private:
  WaveType wave_type_;           // 波形の種類
  float frequency_;              // 周波数（Hz）
  mutable Uint32 noise_state_;   // ノイズジェネレーター状態（LCG用、mutableでconstメソッドから変更可能）
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
   * @brief ADSRをまとめて指定
   */
  void setADSR(float a, float d, float s, float r){
    setAttackTime(a);
    setDecayTime(d);
    setSustainLevel(s);
    setReleaseTime(r);
  }

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
 * コールバック方式でバックグラウンドスレッドから呼び出されます。
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
        gate_(false), note_duration_(0.0f), note_volume_(1.0f),
        master_volume_(1.0f), debug_first_samples_(false) {
    // デフォルトのオシレーターとエンベロープを設定
    oscillator_ = std::make_unique<Oscillator>(WaveType::Sine, 440.0f);
    envelope_ = std::make_unique<Envelope>(0.01f, 0.1f, 0.7f, 0.2f);

    // オーディオストリームを初期化（コールバック方式）
    SDL_AudioSpec spec;
    spec.channels = 1;           // モノラル
    spec.format = SDL_AUDIO_F32; // 32ビット浮動小数点
    spec.freq = sample_rate_;

    // コールバック方式でオーディオストリームを開く
    stream_ = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &spec,
        audioCallback,  // コールバック関数
        this);          // ユーザーデータ（this）

    if (!stream_) {
      SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio device: %s", SDL_GetError());
      return;
    }

    SYNTH_LOG("Audio stream initialized (callback mode): %p, sample_rate=%d", stream_, sample_rate_);

    // オーディオデバイスを再開（デフォルトは一時停止状態）
    if (!SDL_ResumeAudioStreamDevice(stream_)) {
      SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to resume audio device: %s", SDL_GetError());
    } else {
      SYNTH_LOG("Audio device resumed successfully (callback mode)");
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
   * @brief マスターボリュームを設定
   * @param volume ボリューム（0.0〜1.0）
   */
  void setVolume(float volume) {
    master_volume_ = SDL_clamp(volume, 0.0f, 1.0f);
  }

  /**
   * @brief マスターボリュームを取得
   * @return ボリューム（0.0〜1.0）
   */
  float getVolume() const { return master_volume_; }

  /**
   * @brief ノートオン（音を鳴らし始める）
   * @param frequency 周波数（Hz）
   * @param duration 音の長さ（秒）、0.0なら無限（手動でnoteOff()が必要）
   * @param volume ノート単位のボリューム（0.0〜1.0）、デフォルトは1.0
   */
  void noteOn(float frequency, float duration = 0.0f, float volume = 1.0f) {
    if (!stream_) {
      SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Audio stream not initialized");
      return;
    }

    oscillator_->setFrequency(frequency);
    current_sample_ = 0;  // サンプル位置をリセット
    note_on_time_ = 0.0f;
    note_off_time_ = 0.0f;
    note_duration_ = duration;
    note_volume_ = SDL_clamp(volume, 0.0f, 1.0f);
    gate_ = true;
    is_playing_ = true;
    debug_first_samples_ = true;  // デバッグログを有効化

    SYNTH_LOG("NoteOn: %.2f Hz, duration: %.2f sec, volume: %.2f, stream=%p",
              frequency, duration, note_volume_, stream_);
  }

  /**
   * @brief ノートオフ（音を止め始める）
   */
  void noteOff() {
    if (gate_) {
      note_off_time_ = getCurrentTime();
      gate_ = false;
      SYNTH_LOG("NoteOff at %.2f sec", note_off_time_);
    }
  }

  /**
   * @brief 再生中かどうか
   * @return 再生中ならtrue
   */
  bool isPlaying() const { return is_playing_; }

  /**
   * @brief 更新（メインループから毎フレーム呼び出す）
   *
   * コールバック方式では、サンプル生成はバックグラウンドスレッドで自動的に行われるため、
   * ここでは状態管理のみを行います。
   */
  void update() {
    if (!is_playing_) return;

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
        SYNTH_LOG("Sound finished (envelope reached 0)");
      }
    }
  }

 private:
  /**
   * @brief オーディオコールバック（静的関数、バックグラウンドスレッドから呼び出される）
   * @param userdata SimpleSynthesizerのインスタンスポインタ
   * @param stream オーディオストリーム
   * @param additional_amount 追加で必要なバイト数
   * @param total_amount 合計で必要なバイト数
   */
  static void SDLCALL audioCallback(void* userdata, SDL_AudioStream* stream,
                                     int additional_amount, int total_amount) {
    SimpleSynthesizer* synth = static_cast<SimpleSynthesizer*>(userdata);

    // 再生中でない場合は無音を出力
    if (!synth->is_playing_) {
      // 無音データを送信
      int samples_needed = additional_amount / sizeof(float);
      if (samples_needed > 0) {
        float* silence = new float[samples_needed];
        SDL_memset(silence, 0, additional_amount);
        SDL_PutAudioStreamData(stream, silence, additional_amount);
        delete[] silence;
      }
      return;
    }

    // 必要なサンプル数を計算
    int samples_needed = additional_amount / sizeof(float);
    if (samples_needed <= 0) return;

    // サンプルを生成
    float* samples = new float[samples_needed];
    synth->generateSamples(samples, samples_needed);

    // ストリームにデータを送信
    SDL_PutAudioStreamData(stream, samples, additional_amount);

    delete[] samples;
  }

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

      // エンベロープとボリュームを適用
      // 最終ボリューム = 波形 × エンベロープ × ノートボリューム × マスターボリューム
      samples[i] = wave * envelope_value * note_volume_ * master_volume_;

      // クリッピング防止
      if (samples[i] > 1.0f) samples[i] = 1.0f;
      if (samples[i] < -1.0f) samples[i] = -1.0f;

      // 最初の数サンプルをログ出力（デバッグ用）
      if (debug_first_samples_ && i < 10) {
        SYNTH_LOG("Sample[%d]: time=%.6f, env=%.4f, phase=%.4f, wave=%.4f, output=%.4f",
                  i, current_time, envelope_value, phase, wave, samples[i]);
      }

      current_sample_++;
    }

    if (debug_first_samples_) {
      debug_first_samples_ = false;
      SYNTH_LOG("Generated %d samples, frequency=%.2f Hz", count, oscillator_->getFrequency());
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
  float note_volume_;      // ノート単位のボリューム（0.0〜1.0）
  float master_volume_;    // マスターボリューム（0.0〜1.0）
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
  static constexpr float beatDuration(float bpm) {
    return 60.0f / bpm;
  }

  /**
   * @brief BPMと音符の分数から音符の長さ（秒）を計算
   * @param bpm BPM（Beats Per Minute）
   * @param note_division 音符の分数（4 = 4分音符、8 = 8分音符、...）
   * @param dotted 付点音符かどうか（1.5倍になる）
   * @return 音符の長さ（秒）
   */
  static constexpr float noteDuration(float bpm, int note_division, bool dotted = false) {
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
  bool is_rest;      // 休符かどうか
  WaveType wave_type; // 音色（オシレーター）
  float volume;      // ボリューム（0.0〜1.0）

  /**
   * @brief デフォルトコンストラクタ（配列初期化用）
   */
  constexpr NoteData()
      : note(Note::C), octave(4), duration(0.0f), is_rest(true),
        wave_type(WaveType::Sine), volume(1.0f) {}

  /**
   * @brief コンストラクタ
   * @param n 音階
   * @param oct オクターブ
   * @param dur 長さ（秒）
   * @param rest 休符かどうか
   * @param wave 音色
   * @param vol ボリューム（0.0〜1.0）
   */
  constexpr NoteData(Note n, int oct, float dur, bool rest = false,
                    WaveType wave = WaveType::Sine, float vol = 1.0f)
      : note(n), octave(oct), duration(dur), is_rest(rest),
        wave_type(wave), volume(vol) {}

  /**
   * @brief 周波数を取得
   * @return 周波数（Hz）
   */
  float getFrequency() const {
    return MusicUtil::noteToFrequency(note, octave);
  }
};

/**
 * @brief 固定長音符シーケンス（constexpr対応）
 *
 * std::arrayベースの固定長コンテナで、完全にconstexpr変数として保存可能。
 * MMLパーサーから返され、コンパイル時定数として使用できる。
 */
class FixedNoteSequence {
 public:
  static constexpr size_t MAX_NOTES = 256;  // 最大音符数

  /**
   * @brief デフォルトコンストラクタ
   */
  constexpr FixedNoteSequence() : size_(0), notes_{} {}

  /**
   * @brief 音符を追加
   * @param note 音符データ
   * @return 追加に成功したらtrue
   */
  constexpr bool push_back(const NoteData& note) {
    if (size_ >= MAX_NOTES) return false;
    notes_[size_++] = note;
    return true;
  }

  /**
   * @brief 音符数を取得
   * @return 音符数
   */
  constexpr size_t size() const { return size_; }

  /**
   * @brief 空かどうか
   * @return 空ならtrue
   */
  constexpr bool empty() const { return size_ == 0; }

  /**
   * @brief インデックスアクセス
   * @param index インデックス
   * @return 音符データへの参照
   */
  constexpr const NoteData& operator[](size_t index) const { return notes_[index]; }

  /**
   * @brief データへのポインタを取得
   * @return データの先頭ポインタ
   */
  constexpr const NoteData* data() const { return notes_.data(); }

  /**
   * @brief イテレータの開始位置
   */
  constexpr const NoteData* begin() const { return notes_.data(); }

  /**
   * @brief イテレータの終了位置
   */
  constexpr const NoteData* end() const { return notes_.data() + size_; }

 private:
  size_t size_;
  std::array<NoteData, MAX_NOTES> notes_;
};

/**
 * @brief MML（Music Macro Language）パーサー
 *
 * MML文字列を解析してNoteDataのリストに変換します。
 * C++20のconstexprに対応しており、コンパイル時にMMLをパースできます。
 *
 * サポートするMML記法:
 * - c, d, e, f, g, a, b: ドレミファソラシ
 * - +, #: シャープ（半音上げる）
 * - -: フラット（半音下げる）
 * - 数字: 音符の長さ（1=全音符、2=2分音符、4=4分音符、8=8分音符など）
 * - o数字: オクターブ指定（o4など）
 * - r: 休符
 * - l数字: デフォルトの音符長を設定
 * - t数字: テンポ（BPM）を設定
 * - @数字: 音色を設定（0=サイン波、1=矩形波、2=ノコギリ波）
 * - v数字: ボリューム設定（v0〜v15で0.0〜1.0にマップ）
 * - .: 付点音符（音符の長さを1.5倍に）
 * - <: オクターブを1つ下げる
 * - >: オクターブを1つ上げる
 *
 * 例: "t120 o4 l4 cdefgab >c"
 */
class MMLParser {
 public:
  /**
   * @brief MML文字列を解析（constexpr対応）
   * @param mml MML文字列
   * @return 音符データのリスト（固定長配列）
   */
  static constexpr FixedNoteSequence parse(const std::string& mml) {
    FixedNoteSequence result;

    // デフォルト値
    float bpm = 120.0f;
    int default_length = 4;  // 4分音符
    int octave = 4;
    WaveType wave_type = WaveType::Sine;
    float volume = 1.0f;     // ボリューム（0.0〜1.0）

    size_t i = 0;
    while (i < mml.length()) {
      char c = constexpr_tolower(mml[i]);

      // 空白はスキップ
      if (constexpr_isspace(c)) {
        i++;
        continue;
      }

      // テンポ設定 (t120など)
      if (c == 't') {
        i++;
        int tempo = parseNumber(mml, i);
        if (tempo > 0) bpm = static_cast<float>(tempo);
        continue;
      }

      // デフォルト音符長設定 (l4など)
      if (c == 'l') {
        i++;
        int length = parseNumber(mml, i);
        if (length > 0) default_length = length;
        continue;
      }

      // オクターブ設定 (o4など)
      if (c == 'o') {
        i++;
        int oct = parseNumber(mml, i);
        if (oct >= 0 && oct <= 8) octave = oct;
        continue;
      }

      // 音色設定 (@0など)
      if (c == '@') {
        i++;
        int wave = parseNumber(mml, i);
        if (wave == 0) wave_type = WaveType::Sine;
        else if (wave == 1) wave_type = WaveType::Square;
        else if (wave == 2) wave_type = WaveType::Sawtooth;
        else if (wave == 3) wave_type = WaveType::Noise;
        continue;
      }

      // ボリューム設定 (v0〜v15など)
      if (c == 'v') {
        i++;
        int vol = parseNumber(mml, i);
        // v0〜v15を0.0〜1.0にマップ
        volume = static_cast<float>(vol) / 15.0f;
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;
        continue;
      }

      // オクターブ上げ
      if (c == '>') {
        if (octave < 8) octave++;
        i++;
        continue;
      }

      // オクターブ下げ
      if (c == '<') {
        if (octave > 0) octave--;
        i++;
        continue;
      }

      // 休符 (r4など)
      if (c == 'r') {
        i++;
        int length = default_length;
        if (i < mml.length() && constexpr_isdigit(mml[i])) {
          length = parseNumber(mml, i);
        }
        bool dotted = false;
        if (i < mml.length() && mml[i] == '.') {
          dotted = true;
          i++;
        }
        float duration = MusicUtil::noteDuration(bpm, length, dotted);
        result.push_back(NoteData(Note::C, 0, duration, true, wave_type, volume));
        continue;
      }

      // 音符 (c4, d+8など)
      if (c >= 'a' && c <= 'g') {
        Note note = charToNote(c);
        i++;

        // シャープ/フラット
        if (i < mml.length() && (mml[i] == '+' || mml[i] == '#')) {
          note = static_cast<Note>((static_cast<int>(note) + 1) % 12);
          i++;
        } else if (i < mml.length() && mml[i] == '-') {
          note = static_cast<Note>((static_cast<int>(note) + 11) % 12);
          i++;
        }

        // 音符の長さ
        int length = default_length;
        if (i < mml.length() && constexpr_isdigit(mml[i])) {
          length = parseNumber(mml, i);
        }

        // 付点音符
        bool dotted = false;
        if (i < mml.length() && mml[i] == '.') {
          dotted = true;
          i++;
        }

        float duration = MusicUtil::noteDuration(bpm, length, dotted);
        result.push_back(NoteData(note, octave, duration, false, wave_type, volume));
        continue;
      }

      // 未知の文字はスキップ
      i++;
    }

    return result;
  }

 private:
  /**
   * @brief constexpr版のtolower
   */
  static constexpr char constexpr_tolower(char c) {
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
  }

  /**
   * @brief constexpr版のisspace
   */
  static constexpr bool constexpr_isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
  }

  /**
   * @brief constexpr版のisdigit
   */
  static constexpr bool constexpr_isdigit(char c) {
    return c >= '0' && c <= '9';
  }

  /**
   * @brief 文字を音階に変換
   */
  static constexpr Note charToNote(char c) {
    switch (c) {
      case 'c': return Note::C;
      case 'd': return Note::D;
      case 'e': return Note::E;
      case 'f': return Note::F;
      case 'g': return Note::G;
      case 'a': return Note::A;
      case 'b': return Note::B;
      default: return Note::C;
    }
  }

  /**
   * @brief 文字列から数値を解析
   */
  static constexpr int parseNumber(const std::string& str, size_t& pos) {
    int result = 0;
    while (pos < str.length() && constexpr_isdigit(str[pos])) {
      result = result * 10 + (str[pos] - '0');
      pos++;
    }
    return result;
  }
};

/**
 * @brief MMLユーザー定義リテラル（constexpr対応）
 *
 * 使用例:
 *   constexpr auto notes = "t120 o4 cdefgab"_mml;  // コンパイル時評価（完全なconstexpr）
 *   auto notes = "t120 o4 cdefgab"_mml;            // 実行時評価も可能
 */
constexpr FixedNoteSequence operator""_mml(const char* str, size_t len) {
  return MMLParser::parse(std::string(str, len));
}

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
      : synthesizer_(synthesizer), bpm_(bpm), volume_(1.0f),
        current_note_index_(0), is_playing_(false), sequence_time_(0.0f),
        last_update_time_(0), loop_enabled_(false), loop_count_(-1),
        current_loop_(0), timer_id_(0), update_interval_ns_(15000000) {
    // デフォルト: 32分音符相当の精度（15ms = 15,000,000ns間隔）
  }

  /**
   * @brief デストラクタ
   */
  ~Sequencer() {
    stopTimer();
  }

  /**
   * @brief シーケンサーの更新間隔を設定（ナノ秒）
   * @param interval_ns 更新間隔（ナノ秒）、小さいほど精度が高い
   */
  void setUpdateIntervalNS(Uint64 interval_ns) {
    update_interval_ns_ = interval_ns;
  }

  /**
   * @brief シーケンサーの更新間隔を設定（ミリ秒、利便性のため）
   * @param interval_ms 更新間隔（ミリ秒）
   */
  void setUpdateInterval(Uint32 interval_ms) {
    update_interval_ns_ = static_cast<Uint64>(interval_ms) * 1000000;  // ms to ns
  }

  /**
   * @brief シーケンサーの更新間隔を取得（ナノ秒）
   * @return 更新間隔（ナノ秒）
   */
  Uint64 getUpdateIntervalNS() const {
    return update_interval_ns_;
  }

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
  void setVolume(float volume) {
    volume_ = SDL_clamp(volume, 0.0f, 1.0f);
  }

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
  void setLoop(bool enabled, int count = -1) {
    loop_enabled_ = enabled;
    loop_count_ = count;
  }

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
   * @param wave_type 音色（オシレーター）
   * @param volume ボリューム（0.0〜1.0）
   */
  void addNote(Note note, int octave, int note_division, bool dotted = false,
               WaveType wave_type = WaveType::Sine, float volume = 1.0f) {
    float duration = MusicUtil::noteDuration(bpm_, note_division, dotted);
    sequence_.emplace_back(note, octave, duration, false, wave_type, volume);
  }

  /**
   * @brief 休符を追加
   * @param note_division 音符の分数（4 = 4分音符）
   * @param dotted 付点音符かどうか
   */
  void addRest(int note_division, bool dotted = false) {
    float duration = MusicUtil::noteDuration(bpm_, note_division, dotted);
    // is_restフラグで休符を表現
    sequence_.emplace_back(Note::C, 0, duration, true, WaveType::Sine, 1.0f);
  }

  /**
   * @brief MMLから生成された音符シーケンスを設定（std::vector版）
   * @param notes 音符データのリスト
   */
  void setSequence(std::vector<NoteData>&& notes) {
    sequence_ = std::move(notes);
  }

  /**
   * @brief MMLから生成された音符シーケンスを設定（std::vector版）
   * @param notes 音符データのリスト
   */
  void setSequence(const std::vector<NoteData>& notes) {
    sequence_ = notes;
  }

  /**
   * @brief MMLから生成された音符シーケンスを設定（FixedNoteSequence版）
   * @param notes 固定長音符シーケンス（constexpr対応）
   */
  void setSequence(const FixedNoteSequence& notes) {
    sequence_.clear();
    sequence_.reserve(notes.size());
    for (const auto& note : notes) {
      sequence_.push_back(note);
    }
  }

  /**
   * @brief 任意のコンテナから音符シーケンスを設定（std::span版）
   * @param notes 音符データのスパン（配列、vector、FixedNoteSequenceなど）
   */
  void setSequence(std::span<const NoteData> notes) {
    sequence_.clear();
    sequence_.reserve(notes.size());
    for (const auto& note : notes) {
      sequence_.push_back(note);
    }
  }

  /**
   * @brief シーケンスを再生開始
   */
  void play() {
    if (sequence_.empty()) return;

    current_note_index_ = 0;
    sequence_time_ = 0.0f;
    current_loop_ = 0;
    is_playing_ = true;
    last_update_time_ = SDL_GetTicks();

    // 最初の音符を再生
    playCurrentNote();

    // 高精度タイマーを開始
    startTimer();
  }

  /**
   * @brief シーケンスを停止
   */
  void stop() {
    is_playing_ = false;
    synthesizer_->noteOff();
    stopTimer();
  }

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
  void update() {
    // タイマーベースの更新に移行したため、このメソッドは空にする
    // 必要に応じて状態確認などの軽量処理のみ行う
  }

 private:
  /**
   * @brief タイマーコールバック（静的関数、SDL_AddTimerNS用）
   * @param userdata Sequencerインスタンスへのポインタ
   * @param timerID タイマーID
   * @param interval タイマー間隔（ナノ秒）
   * @return 次のタイマー間隔（0で停止）
   */
  static Uint64 SDLCALL timerCallback(void* userdata, SDL_TimerID timerID, Uint64 interval) {
    Sequencer* sequencer = static_cast<Sequencer*>(userdata);
    if (sequencer) {
      sequencer->internalUpdate();
    }
    return interval;  // 同じ間隔で継続
  }

  /**
   * @brief タイマーを開始
   */
  void startTimer() {
    if (timer_id_ != 0) {
      stopTimer();  // 既存のタイマーを停止
    }
    timer_id_ = SDL_AddTimerNS(update_interval_ns_, timerCallback, this);
    if (timer_id_ == 0) {
      SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to create timer: %s", SDL_GetError());
    }
  }

  /**
   * @brief タイマーを停止
   */
  void stopTimer() {
    if (timer_id_ != 0) {
      SDL_RemoveTimer(timer_id_);
      timer_id_ = 0;
    }
  }

  /**
   * @brief 内部更新処理（タイマーから呼ばれる）
   */
  void internalUpdate() {
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
          handleSequenceEnd();
        }
      }
    }
  }

  /**
   * @brief シーケンス終了時の処理
   */
  void handleSequenceEnd() {
    if (!loop_enabled_) {
      // ループが無効の場合は停止
      is_playing_ = false;
      return;
    }

    // ループ回数をチェック
    if (loop_count_ >= 0) {
      // 回数指定ループの場合
      current_loop_++;
      if (current_loop_ > loop_count_) {
        // 指定回数に達したので停止
        is_playing_ = false;
        return;
      }
    }
    // loop_count_ == -1の場合は無限ループ

    // シーケンスを最初から再生
    current_note_index_ = 0;
    sequence_time_ = 0.0f;
    playCurrentNote();
  }

  /**
   * @brief 現在の音符を再生
   */
  void playCurrentNote() {
    if (current_note_index_ >= sequence_.size()) return;

    const NoteData& note_data = sequence_[current_note_index_];

    // 休符の場合は何もしない
    if (note_data.is_rest) {
      synthesizer_->noteOff();
      return;
    }

    // 音色を設定
    synthesizer_->getOscillator().setWaveType(note_data.wave_type);

    // ボリュームを計算（シーケンサーボリューム × ノートボリューム）
    float final_volume = volume_ * note_data.volume;

    // 音符を再生
    float frequency = note_data.getFrequency();
    synthesizer_->noteOn(frequency, note_data.duration, final_volume);
  }

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
   */
  explicit MultiTrackSequencer(size_t track_count, int sample_rate = 44100, float bpm = 120.0f)
      : track_count_(track_count), bpm_(bpm), master_volume_(1.0f), is_paused_(false) {
    // トラック数分のシンセサイザーとシーケンサーを生成
    for (size_t i = 0; i < track_count; ++i) {
      auto synth = std::make_unique<SimpleSynthesizer>(sample_rate);
      auto seq = std::make_unique<Sequencer>(synth.get(), bpm);

      synthesizers_.push_back(std::move(synth));
      sequencers_.push_back(std::move(seq));
    }
  }

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
  SimpleSynthesizer* getSynthesizer(size_t track_index) {
    if (track_index >= track_count_) return nullptr;
    return synthesizers_[track_index].get();
  }

  /**
   * @brief 指定トラックのシーケンサーを取得
   * @param track_index トラックインデックス
   * @return シーケンサーへのポインタ（範囲外の場合はnullptr）
   */
  Sequencer* getSequencer(size_t track_index) {
    if (track_index >= track_count_) return nullptr;
    return sequencers_[track_index].get();
  }

  /**
   * @brief 指定トラックにシーケンスを設定
   * @param track_index トラックインデックス
   * @param notes 音符シーケンス
   */
  void setTrackSequence(size_t track_index, const FixedNoteSequence& notes) {
    if (track_index >= track_count_) return;
    sequencers_[track_index]->clear();
    sequencers_[track_index]->setSequence(notes);
  }

  /**
   * @brief 指定トラックにシーケンスを設定（std::vector版）
   * @param track_index トラックインデックス
   * @param notes 音符シーケンス
   */
  void setTrackSequence(size_t track_index, const std::vector<NoteData>& notes) {
    if (track_index >= track_count_) return;
    sequencers_[track_index]->clear();
    sequencers_[track_index]->setSequence(notes);
  }

  /**
   * @brief マスターボリュームを設定（全トラックに適用）
   * @param volume ボリューム（0.0〜1.0）
   */
  void setMasterVolume(float volume) {
    master_volume_ = SDL_clamp(volume, 0.0f, 1.0f);
    for (auto& synth : synthesizers_) {
      synth->setVolume(master_volume_);
    }
  }

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
  void setLoop(bool enabled, int count = -1) {
    for (auto& seq : sequencers_) {
      seq->setLoop(enabled, count);
    }
  }

  /**
   * @brief シーケンサーの更新間隔を設定（ナノ秒）
   * @param interval_ns 更新間隔（ナノ秒）、小さいほど精度が高い
   */
  void setUpdateIntervalNS(Uint64 interval_ns) {
    for (auto& seq : sequencers_) {
      seq->setUpdateIntervalNS(interval_ns);
    }
  }

  /**
   * @brief シーケンサーの更新間隔を設定（ミリ秒、利便性のため）
   * @param interval_ms 更新間隔（ミリ秒）
   */
  void setUpdateInterval(Uint32 interval_ms) {
    for (auto& seq : sequencers_) {
      seq->setUpdateInterval(interval_ms);
    }
  }

  /**
   * @brief 全トラックを再生開始
   */
  void play() {
    is_paused_ = false;
    for (auto& seq : sequencers_) {
      seq->play();
    }
  }

  /**
   * @brief 全トラックを停止
   */
  void stop() {
    is_paused_ = false;
    for (auto& seq : sequencers_) {
      seq->stop();
    }
  }

  /**
   * @brief 全トラックを一時停止
   */
  void pause() {
    if (!is_paused_) {
      is_paused_ = true;
      for (auto& seq : sequencers_) {
        seq->stop();  // 現在はstopで代用（将来的にpause実装）
      }
    }
  }

  /**
   * @brief 全トラックを再開
   */
  void resume() {
    if (is_paused_) {
      is_paused_ = false;
      for (auto& seq : sequencers_) {
        seq->play();  // 現在はplayで代用（将来的にresume実装）
      }
    }
  }

  /**
   * @brief 再生中かどうか
   * @return 少なくとも1トラックが再生中ならtrue
   */
  bool isPlaying() const {
    for (const auto& seq : sequencers_) {
      if (seq->isPlaying()) return true;
    }
    return false;
  }

  /**
   * @brief 一時停止中かどうか
   * @return 一時停止中ならtrue
   */
  bool isPaused() const { return is_paused_; }

  /**
   * @brief 更新（メインループから毎フレーム呼び出す）
   */
  void update() {
    for (auto& synth : synthesizers_) {
      synth->update();
    }
    for (auto& seq : sequencers_) {
      seq->update();
    }
  }

 private:
  size_t track_count_;
  float bpm_;
  float master_volume_;
  bool is_paused_;
  std::vector<std::unique_ptr<SimpleSynthesizer>> synthesizers_;
  std::vector<std::unique_ptr<Sequencer>> sequencers_;
};

/**
 * @brief BGMマネージャー
 *
 * 複数の楽曲（MultiTrackSequencer）を管理し、切り替える。
 */
class BGMManager {
 public:
  /**
   * @brief BGMを登録
   * @param id BGM ID
   * @param bgm マルチトラックシーケンサー
   */
  void registerBGM(const std::string& id, std::unique_ptr<MultiTrackSequencer> bgm) {
    bgm_map_[id] = std::move(bgm);
  }

  /**
   * @brief BGMを取得
   * @param id BGM ID
   * @return マルチトラックシーケンサーへのポインタ（存在しない場合はnullptr）
   */
  MultiTrackSequencer* getBGM(const std::string& id) {
    auto it = bgm_map_.find(id);
    if (it != bgm_map_.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  /**
   * @brief BGMを再生
   * @param id BGM ID
   * @return 成功したらtrue
   */
  bool play(const std::string& id) {
    auto* bgm = getBGM(id);
    if (!bgm) return false;

    // 現在再生中のBGMを停止
    if (!current_bgm_id_.empty() && current_bgm_id_ != id) {
      stop();
    }

    current_bgm_id_ = id;
    bgm->play();
    return true;
  }

  /**
   * @brief 現在のBGMを停止
   */
  void stop() {
    if (!current_bgm_id_.empty()) {
      auto* bgm = getBGM(current_bgm_id_);
      if (bgm) {
        bgm->stop();
      }
      current_bgm_id_.clear();
    }
  }

  /**
   * @brief 現在のBGMを一時停止
   */
  void pause() {
    if (!current_bgm_id_.empty()) {
      auto* bgm = getBGM(current_bgm_id_);
      if (bgm) {
        bgm->pause();
      }
    }
  }

  /**
   * @brief 現在のBGMを再開
   */
  void resume() {
    if (!current_bgm_id_.empty()) {
      auto* bgm = getBGM(current_bgm_id_);
      if (bgm) {
        bgm->resume();
      }
    }
  }

  /**
   * @brief 全BGMのマスターボリュームを設定
   * @param volume ボリューム（0.0〜1.0）
   */
  void setMasterVolume(float volume) {
    master_volume_ = SDL_clamp(volume, 0.0f, 1.0f);
    for (auto& [id, bgm] : bgm_map_) {
      bgm->setMasterVolume(master_volume_);
    }
  }

  /**
   * @brief マスターボリュームを取得
   * @return ボリューム（0.0〜1.0）
   */
  float getMasterVolume() const { return master_volume_; }

  /**
   * @brief 現在再生中のBGM IDを取得
   * @return BGM ID（再生中でない場合は空文字列）
   */
  const std::string& getCurrentBGMId() const { return current_bgm_id_; }

  /**
   * @brief BGMが再生中かどうか
   * @return 再生中ならtrue
   */
  bool isPlaying() const {
    if (current_bgm_id_.empty()) return false;
    auto it = bgm_map_.find(current_bgm_id_);
    if (it != bgm_map_.end()) {
      return it->second->isPlaying();
    }
    return false;
  }

  /**
   * @brief 更新（メインループから毎フレーム呼び出す）
   */
  void update() {
    if (!current_bgm_id_.empty()) {
      auto* bgm = getBGM(current_bgm_id_);
      if (bgm) {
        bgm->update();
      }
    }
  }

 private:
  std::unordered_map<std::string, std::unique_ptr<MultiTrackSequencer>> bgm_map_;
  std::string current_bgm_id_;
  float master_volume_ = 1.0f;
};

}  // namespace MyGame
