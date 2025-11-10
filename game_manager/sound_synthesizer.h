#pragma once

#include <SDL3/SDL.h>
#include <array>
#include <cmath>
#include <memory>
#include <span>
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
        gate_(false), note_duration_(0.0f), debug_first_samples_(false) {
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

    SYNTH_LOG("NoteOn: %.2f Hz, duration: %.2f sec, stream=%p", frequency, duration, stream_);
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

      // エンベロープを適用
      samples[i] = wave * envelope_value;

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

  /**
   * @brief デフォルトコンストラクタ（配列初期化用）
   */
  constexpr NoteData()
      : note(Note::C), octave(4), duration(0.0f), is_rest(true), wave_type(WaveType::Sine) {}

  /**
   * @brief コンストラクタ
   * @param n 音階
   * @param oct オクターブ
   * @param dur 長さ（秒）
   * @param rest 休符かどうか
   * @param wave 音色
   */
  constexpr NoteData(Note n, int oct, float dur, bool rest = false, WaveType wave = WaveType::Sine)
      : note(n), octave(oct), duration(dur), is_rest(rest), wave_type(wave) {}

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
        result.push_back(NoteData(Note::C, 0, duration, true, wave_type));
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
        result.push_back(NoteData(note, octave, duration, false, wave_type));
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
   * @param wave_type 音色（オシレーター）
   */
  void addNote(Note note, int octave, int note_division, bool dotted = false,
               WaveType wave_type = WaveType::Sine) {
    float duration = MusicUtil::noteDuration(bpm_, note_division, dotted);
    sequence_.emplace_back(note, octave, duration, false, wave_type);
  }

  /**
   * @brief 休符を追加
   * @param note_division 音符の分数（4 = 4分音符）
   * @param dotted 付点音符かどうか
   */
  void addRest(int note_division, bool dotted = false) {
    float duration = MusicUtil::noteDuration(bpm_, note_division, dotted);
    // is_restフラグで休符を表現
    sequence_.emplace_back(Note::C, 0, duration, true, WaveType::Sine);
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
    if (note_data.is_rest) {
      synthesizer_->noteOff();
      return;
    }

    // 音色を設定
    synthesizer_->getOscillator().setWaveType(note_data.wave_type);

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
