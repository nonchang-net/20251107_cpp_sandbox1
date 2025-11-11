#pragma once

namespace MySound {

/**
 * @brief オーディオエフェクトの基底クラス
 *
 * 全てのエフェクト（フィルター、モジュレーション、リバーブなど）が
 * 継承する共通インターフェース。
 *
 * エフェクトチェーン（複数のエフェクトを直列接続）を実現するための
 * 統一されたインターフェースを提供します。
 *
 * 使用例:
 * @code
 * std::unique_ptr<AudioEffect> effect = std::make_unique<SomeEffect>();
 * float output = effect->process(input_sample);
 * @endcode
 */
class AudioEffect {
 public:
  /**
   * @brief 仮想デストラクタ
   */
  virtual ~AudioEffect() = default;

  /**
   * @brief サンプルを処理（純粋仮想関数）
   *
   * 入力サンプルに対してエフェクト処理を適用し、出力サンプルを返します。
   * 各エフェクトクラスで実装が必要です。
   *
   * @param input 入力サンプル
   * @return エフェクト処理後の出力サンプル
   */
  virtual float process(float input) = 0;

  /**
   * @brief エフェクト状態をリセット（純粋仮想関数）
   *
   * エフェクトの内部状態（遅延バッファ、LFOの位相など）をリセットします。
   * 音が切り替わる際などに呼び出されます。
   */
  virtual void reset() = 0;
};

}  // namespace MySound
