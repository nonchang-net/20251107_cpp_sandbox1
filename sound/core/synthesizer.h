#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include "oscillator.h"
#include "envelope.h"
#include "../sound_constants.h"

namespace MySound {

/**
 * @brief シンプルなシンセサイザー
 *
 * オシレーターとエンベロープを組み合わせて音を生成します。
 * SDL_AudioStreamを使ってリアルタイムで音を出力します。
 */
class SimpleSynthesizer {
 public:
  /**
   * @brief コンストラクタ
   * @param sample_rate サンプリングレート
   */
  explicit SimpleSynthesizer(int sample_rate = DEFAULT_SAMPLE_RATE);

  /**
   * @brief デストラクタ
   */
  ~SimpleSynthesizer();

  // コピー禁止
  SimpleSynthesizer(const SimpleSynthesizer&) = delete;
  SimpleSynthesizer& operator=(const SimpleSynthesizer&) = delete;

  /**
   * @brief オシレーターを取得
   * @return オシレーターへの参照
   */
  Oscillator& getOscillator();

  /**
   * @brief エンベロープを取得
   * @return エンベロープへの参照
   */
  Envelope& getEnvelope();

  /**
   * @brief マスターボリュームを設定
   * @param volume ボリューム（0.0〜1.0）
   */
  void setVolume(float volume);

  /**
   * @brief マスターボリュームを取得
   * @return ボリューム（0.0〜1.0）
   */
  float getVolume() const;

  /**
   * @brief ノートオン（音を鳴らし始める）
   * @param frequency 周波数（Hz）
   * @param duration 音の長さ（秒）、0.0なら無限（手動でnoteOff()が必要）
   * @param volume ノート単位のボリューム（0.0〜1.0）、デフォルトは1.0
   */
  void noteOn(float frequency, float duration = 0.0f, float volume = 1.0f);

  /**
   * @brief ノートオフ（音を止め始める）
   */
  void noteOff();

  /**
   * @brief 更新（毎フレーム呼び出す）
   */
  void update();

  /**
   * @brief サンプリングレートを取得
   * @return サンプリングレート
   */
  int getSampleRate() const;

 private:
  /**
   * @brief オーディオコールバック（静的メソッド）
   * @param userdata ユーザーデータ（SimpleSynthesizerインスタンス）
   * @param stream オーディオストリーム
   * @param additional_amount 追加要求サンプル数
   * @param total_amount 総サンプル数
   */
  static void SDLCALL audioCallback(void* userdata, SDL_AudioStream* stream,
                                    int additional_amount, int total_amount);

  /**
   * @brief サンプルを生成
   * @param samples 出力バッファ
   * @param num_samples サンプル数
   */
  void generateSamples(float* samples, int num_samples);

  /**
   * @brief 現在の時刻を取得（秒）
   * @return 現在の時刻
   */
  float getCurrentTime() const;

  std::unique_ptr<Oscillator> oscillator_;  // オシレーター
  std::unique_ptr<Envelope> envelope_;      // エンベロープ
  SDL_AudioStream* stream_;                 // オーディオストリーム

  int sample_rate_;       // サンプリングレート
  Uint64 current_sample_; // 現在のサンプル位置
  float master_volume_;   // マスターボリューム（0.0〜1.0）
  float note_volume_;     // ノート単位のボリューム（0.0〜1.0）

  bool is_playing_;       // 再生中フラグ
  bool gate_;             // ゲート（ノートオン/オフ）
  float note_on_time_;    // ノートオン時刻（秒）
  float note_off_time_;   // ノートオフ時刻（秒）
  float note_duration_;   // ノートの長さ（秒）、0.0なら無限

  // デバッグ用
  bool debug_first_samples_;
};

}  // namespace MySound
