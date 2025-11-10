#include "oscillator.h"
#include <cmath>

namespace MySound {

Oscillator::Oscillator(WaveType wave_type, float frequency)
    : wave_type_(wave_type), frequency_(frequency), noise_state_(DEFAULT_NOISE_SEED) {}

void Oscillator::setWaveType(WaveType wave_type) {
  wave_type_ = wave_type;
}

void Oscillator::setFrequency(float frequency) {
  frequency_ = frequency;
}

WaveType Oscillator::getWaveType() const {
  return wave_type_;
}

float Oscillator::getFrequency() const {
  return frequency_;
}

void Oscillator::setNoiseSeed(Uint32 seed) {
  noise_state_ = seed;
}

float Oscillator::generate(float phase) const {
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

float Oscillator::generateNoise() const {
  // Linear Congruential Generator
  // 参考: Numerical Recipes推奨パラメータ
  // state = (a * state + c) mod 2^32
  noise_state_ = noise_state_ * LCG_MULTIPLIER + LCG_INCREMENT;

  // uint32_tをint32_tとして解釈し、-1.0〜1.0の範囲に正規化
  // int32_tの範囲は-2^31 ~ 2^31-1なので、2^31で割る
  return static_cast<int32_t>(noise_state_) / 2147483648.0f;
}

}  // namespace MySound
