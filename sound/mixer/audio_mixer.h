#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include "../core/synthesizer.h"
#include "../effect/audio_effect.h"
#include "../sound_constants.h"

namespace MySound {

/**
 * @brief オーディオミキサー
 *
 * 複数のSimpleSynthesizerからサンプルを取得してミックスし、
 * エフェクトチェーンを適用してからオーディオデバイスに出力します。
 *
 * 主な用途:
 * - MultiTrackSequencerでの複数トラックのミキシング
 * - ミックス後のマスターエフェクト適用
 *
 * 使用例:
 * @code
 * // ストリームなしモードでシンセサイザーを作成
 * auto synth1 = std::make_unique<SimpleSynthesizer>(44100, false);
 * auto synth2 = std::make_unique<SimpleSynthesizer>(44100, false);
 *
 * // ミキサーを作成し、シンセサイザーを登録
 * auto mixer = std::make_unique<AudioMixer>(44100);
 * mixer->addSynthesizer(synth1.get());
 * mixer->addSynthesizer(synth2.get());
 *
 * // マスターエフェクトを追加
 * auto reverb = std::make_unique<ReverbEffect>(44100);
 * mixer->addEffect(std::move(reverb));
 * @endcode
 */
class AudioMixer {
 public:
  /**
   * @brief コンストラクタ
   * @param sample_rate サンプリングレート
   * @param num_output_channels 出力チャンネル数（1=モノラル、2=ステレオ、3以上=マルチチャンネル）
   * @param enable_stream オーディオストリームを有効にするか（falseの場合はミキシングのみ）
   */
  explicit AudioMixer(int sample_rate = DEFAULT_SAMPLE_RATE,
                      int num_output_channels = 2,
                      bool enable_stream = true);

  /**
   * @brief デストラクタ
   */
  ~AudioMixer();

  // コピー禁止
  AudioMixer(const AudioMixer&) = delete;
  AudioMixer& operator=(const AudioMixer&) = delete;

  /**
   * @brief シンセサイザーを追加
   *
   * ミキサーに登録されたシンセサイザーは、ミックス時にサンプル生成を要求されます。
   * シンセサイザーはストリームなしモード（enable_stream=false）で作成されている必要があります。
   *
   * @param synth シンセサイザーへのポインタ
   */
  void addSynthesizer(SimpleSynthesizer* synth);

  /**
   * @brief 全てのシンセサイザーを削除
   */
  void clearSynthesizers();

  /**
   * @brief シンセサイザー数を取得
   * @return 登録されているシンセサイザーの数
   */
  size_t getSynthesizerCount() const;

  /**
   * @brief センドレベルを設定
   *
   * 指定されたシンセサイザー（入力チャンネル）から指定された出力チャンネルへの
   * センドレベル（音量）を設定します。
   *
   * @param synth_index シンセサイザーのインデックス（0から始まる）
   * @param output_channel 出力チャンネル番号（0から始まる）
   * @param level センドレベル（0.0〜1.0）
   */
  void setSendLevel(size_t synth_index, size_t output_channel, float level);

  /**
   * @brief センドレベルを取得
   * @param synth_index シンセサイザーのインデックス
   * @param output_channel 出力チャンネル番号
   * @return センドレベル（0.0〜1.0）
   */
  float getSendLevel(size_t synth_index, size_t output_channel) const;

  /**
   * @brief ステレオパンを設定（2チャンネル出力専用）
   *
   * ステレオ出力（2チャンネル）の場合のパン設定のヘルパーメソッドです。
   * 内部的には各出力チャンネルへのセンドレベルとして設定されます。
   *
   * @param synth_index シンセサイザーのインデックス
   * @param pan パン値（-1.0=左、0.0=中央、1.0=右）
   */
  void setPan(size_t synth_index, float pan);

  /**
   * @brief エフェクトを追加
   *
   * エフェクトをエフェクトチェーンの末尾に追加します。
   * エフェクトの所有権はミキサーに移動します。
   * ミックス後のサンプルに対して、追加順に直列に処理されます。
   *
   * @param effect 追加するエフェクト（所有権を移動）
   */
  void addEffect(std::unique_ptr<AudioEffect> effect);

  /**
   * @brief 全てのエフェクトを削除
   */
  void clearEffects();

  /**
   * @brief エフェクト数を取得
   * @return 現在設定されているエフェクトの数
   */
  size_t getEffectCount() const;

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
   * @brief サンプリングレートを取得
   * @return サンプリングレート
   */
  int getSampleRate() const;

  /**
   * @brief 出力チャンネル数を取得
   * @return 出力チャンネル数
   */
  int getNumOutputChannels() const;

  /**
   * @brief サンプルを生成（ストリームなしモード用）
   *
   * ストリームなしモードで作成されたAudioMixerの場合、
   * この関数を明示的に呼び出してミックスされたサンプルを取得します。
   *
   * @param samples 出力バッファ
   * @param num_samples サンプル数
   */
  void generateSamples(float* samples, int num_samples);

 private:
  /**
   * @brief オーディオコールバック（静的メソッド）
   * @param userdata ユーザーデータ（AudioMixerインスタンス）
   * @param stream オーディオストリーム
   * @param additional_amount 追加要求サンプル数
   * @param total_amount 総サンプル数
   */
  static void SDLCALL audioCallback(void* userdata, SDL_AudioStream* stream,
                                    int additional_amount, int total_amount);

  /**
   * @brief サンプルをミックス
   *
   * 登録された全てのシンセサイザーからサンプルを取得し、
   * ミックスしてからエフェクトチェーンを適用します。
   *
   * @param output 出力バッファ
   * @param num_samples サンプル数
   */
  void mixSamples(float* output, int num_samples);

  std::vector<SimpleSynthesizer*> synthesizers_;       // シンセサイザーリスト（参照のみ）
  std::vector<std::unique_ptr<AudioEffect>> effects_;  // エフェクトチェーン
  SDL_AudioStream* stream_;                            // オーディオストリーム

  int sample_rate_;                    // サンプリングレート
  int num_output_channels_;            // 出力チャンネル数
  float master_volume_;                // マスターボリューム（0.0〜1.0）
  std::vector<std::vector<float>> send_levels_;  // センドレベル[synth_index][output_channel]
};

}  // namespace MySound
