#include "volume_modulation.h"
#include <SDL3/SDL.h>
#include <cmath>

namespace MySound {

VolumeModulation::VolumeModulation(int sample_rate)
    : sample_rate_(sample_rate),
      rate_(5.0f),  // デフォルト: 5Hz
      depth_(0.5f), // デフォルト: 50%
      lfo_(std::make_unique<Oscillator>(WaveType::Sine, 5.0f)),
      current_sample_(0) {
}

void VolumeModulation::setRate(float rate) {
  // レートを制限（0.1Hz〜20.0Hz）
  rate_ = SDL_clamp(rate, 0.1f, 20.0f);
  lfo_->setFrequency(rate_);
}

void VolumeModulation::setDepth(float depth) {
  // 深さを制限（0.0〜1.0）
  depth_ = SDL_clamp(depth, 0.0f, 1.0f);
}

void VolumeModulation::setWaveType(WaveType wave_type) {
  lfo_->setWaveType(wave_type);
}

WaveType VolumeModulation::getWaveType() const {
  return lfo_->getWaveType();
}

float VolumeModulation::process(float input) {
  // LFOの位相を計算（0.0〜1.0）
  float phase = SDL_fmodf(
      static_cast<float>(current_sample_) * rate_ / sample_rate_, 1.0f);

  // LFOの波形を生成（-1.0〜1.0）
  float lfo = lfo_->generate(phase);

  // LFOを0.0〜1.0の範囲にマップ（ユニポーラ）
  float lfo_unipolar = (lfo + 1.0f) / 2.0f;

  // モジュレーション量を計算
  // depth=0.0: ボリューム変化なし（modulation=1.0）
  // depth=1.0: LFOの波形に完全に従う（modulation=0.0〜1.0）
  float modulation = 1.0f - depth_ + depth_ * lfo_unipolar;

  current_sample_++;

  return input * modulation;
}

void VolumeModulation::reset() {
  current_sample_ = 0;
}

}  // namespace MySound
