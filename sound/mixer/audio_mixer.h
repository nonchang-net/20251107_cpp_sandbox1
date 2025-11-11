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
   */
  explicit AudioMixer(int sample_rate = DEFAULT_SAMPLE_RATE);

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

  int sample_rate_;       // サンプリングレート
  float master_volume_;   // マスターボリューム（0.0〜1.0）
};

}  // namespace MySound
