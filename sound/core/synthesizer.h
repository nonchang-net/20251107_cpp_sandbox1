#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include "oscillator.h"
#include "envelope.h"
#include "../effect/biquad_filter.h"
#include "../effect/volume_modulation.h"
#include "../sound_constants.h"

namespace MySound {

// 前方宣言
class BiquadFilter;
class VolumeModulation;

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

  /**
   * @brief フィルターを有効化
   *
   * フィルターを有効化します。デフォルトはLowpassフィルター（1000Hz, Q=1.0）です。
   * getFilter()でフィルターのパラメータを調整できます。
   */
  void enableFilter();

  /**
   * @brief フィルターを無効化
   *
   * フィルターを無効化します。無効化時は処理コストがほぼゼロになります。
   */
  void disableFilter();

  /**
   * @brief フィルターが有効かどうか
   * @return フィルターが有効ならtrue
   */
  bool isFilterEnabled() const;

  /**
   * @brief フィルターを取得
   *
   * フィルターのパラメータを調整するためのアクセサ。
   * フィルターが無効の場合はnullptrを返します。
   *
   * @return フィルターへのポインタ（無効時はnullptr）
   */
  BiquadFilter* getFilter();

  /**
   * @brief ボリュームモジュレーション（トレモロ）を有効化
   *
   * ボリュームモジュレーションを有効化します。
   * デフォルトは5Hz、50%の深さ、サイン波です。
   * getVolumeModulation()でパラメータを調整できます。
   */
  void enableVolumeModulation();

  /**
   * @brief ボリュームモジュレーション（トレモロ）を無効化
   *
   * ボリュームモジュレーションを無効化します。無効化時は処理コストがほぼゼロになります。
   */
  void disableVolumeModulation();

  /**
   * @brief ボリュームモジュレーションが有効かどうか
   * @return ボリュームモジュレーションが有効ならtrue
   */
  bool isVolumeModulationEnabled() const;

  /**
   * @brief ボリュームモジュレーションを取得
   *
   * ボリュームモジュレーションのパラメータを調整するためのアクセサ。
   * ボリュームモジュレーションが無効の場合はnullptrを返します。
   *
   * @return ボリュームモジュレーションへのポインタ（無効時はnullptr）
   */
  VolumeModulation* getVolumeModulation();

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

  std::unique_ptr<Oscillator> oscillator_;          // オシレーター
  std::unique_ptr<Envelope> envelope_;              // エンベロープ
  std::unique_ptr<BiquadFilter> filter_;            // Biquadフィルター（nullptrなら無効）
  std::unique_ptr<VolumeModulation> volume_mod_;    // ボリュームモジュレーション（nullptrなら無効）
  SDL_AudioStream* stream_;                         // オーディオストリーム

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
