#include "biquad_filter.h"
#include <cmath>

namespace MySound {

BiquadFilter::BiquadFilter(int sample_rate)
    : sample_rate_(sample_rate),
      type_(BiquadFilterType::Lowpass),
      frequency_(1000.0f),
      q_(1.0f),
      gain_(0.0f),
      detune_(0.0f),
      b0_(1.0f), b1_(0.0f), b2_(0.0f),
      a0_(1.0f), a1_(0.0f), a2_(0.0f),
      x1_(0.0f), x2_(0.0f),
      y1_(0.0f), y2_(0.0f) {
  updateCoefficients();
}

void BiquadFilter::setType(BiquadFilterType type) {
  if (type_ != type) {
    type_ = type;
    updateCoefficients();
  }
}

void BiquadFilter::setFrequency(float frequency) {
  // 周波数範囲を制限（10Hz 〜 Nyquist周波数）
  float nyquist = sample_rate_ / 2.0f;
  frequency = SDL_clamp(frequency, 10.0f, nyquist - 1.0f);

  if (frequency_ != frequency) {
    frequency_ = frequency;
    updateCoefficients();
  }
}

void BiquadFilter::setQ(float q) {
  // Q値を制限
  q = SDL_clamp(q, 0.0001f, 1000.0f);

  if (q_ != q) {
    q_ = q;
    updateCoefficients();
  }
}

void BiquadFilter::setGain(float gain) {
  // ゲインを制限（±40dB）
  gain = SDL_clamp(gain, -40.0f, 40.0f);

  if (gain_ != gain) {
    gain_ = gain;
    updateCoefficients();
  }
}

void BiquadFilter::setDetune(float detune) {
  // デチューンを制限（±1200セント = ±1オクターブ）
  detune = SDL_clamp(detune, -1200.0f, 1200.0f);

  if (detune_ != detune) {
    detune_ = detune;
    updateCoefficients();
  }
}

float BiquadFilter::process(float input) {
  // Direct Form I implementation
  // y[n] = (b0/a0)*x[n] + (b1/a0)*x[n-1] + (b2/a0)*x[n-2]
  //                     - (a1/a0)*y[n-1] - (a2/a0)*y[n-2]

  float output = (b0_ / a0_) * input +
                 (b1_ / a0_) * x1_ +
                 (b2_ / a0_) * x2_ -
                 (a1_ / a0_) * y1_ -
                 (a2_ / a0_) * y2_;

  // 状態を更新
  x2_ = x1_;
  x1_ = input;
  y2_ = y1_;
  y1_ = output;

  return output;
}

void BiquadFilter::reset() {
  x1_ = x2_ = 0.0f;
  y1_ = y2_ = 0.0f;
}

float BiquadFilter::getDetunedFrequency() const {
  // デチューン適用: frequency * 2^(detune/1200)
  // 1200セント = 1オクターブ = 2倍
  if (detune_ == 0.0f) {
    return frequency_;
  }
  return frequency_ * SDL_powf(2.0f, detune_ / 1200.0f);
}

void BiquadFilter::updateCoefficients() {
  // Robert Bristow-Johnson "Cookbook formulae for audio EQ biquad filter coefficients"
  // (404? claude codeが書いたURL……) http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
  // (こっちに数式が揃っている) https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html

  float freq = getDetunedFrequency();
  float w0 = 2.0f * SDL_PI_F * freq / sample_rate_;
  float cos_w0 = SDL_cosf(w0);
  float sin_w0 = SDL_sinf(w0);
  float alpha = sin_w0 / (2.0f * q_);

  // ゲインをリニアに変換（dB → リニア）
  float A = SDL_powf(10.0f, gain_ / 40.0f);  // gain/40 because sqrt(A) = 10^(gain/40)

  switch (type_) {
    case BiquadFilterType::Lowpass:
      b0_ = (1.0f - cos_w0) / 2.0f;
      b1_ = 1.0f - cos_w0;
      b2_ = (1.0f - cos_w0) / 2.0f;
      a0_ = 1.0f + alpha;
      a1_ = -2.0f * cos_w0;
      a2_ = 1.0f - alpha;
      break;

    case BiquadFilterType::Highpass:
      b0_ = (1.0f + cos_w0) / 2.0f;
      b1_ = -(1.0f + cos_w0);
      b2_ = (1.0f + cos_w0) / 2.0f;
      a0_ = 1.0f + alpha;
      a1_ = -2.0f * cos_w0;
      a2_ = 1.0f - alpha;
      break;

    case BiquadFilterType::Bandpass:
      // constant skirt gain, peak gain = Q
      b0_ = alpha;
      b1_ = 0.0f;
      b2_ = -alpha;
      a0_ = 1.0f + alpha;
      a1_ = -2.0f * cos_w0;
      a2_ = 1.0f - alpha;
      break;

    case BiquadFilterType::Notch:
      b0_ = 1.0f;
      b1_ = -2.0f * cos_w0;
      b2_ = 1.0f;
      a0_ = 1.0f + alpha;
      a1_ = -2.0f * cos_w0;
      a2_ = 1.0f - alpha;
      break;

    case BiquadFilterType::Allpass:
      b0_ = 1.0f - alpha;
      b1_ = -2.0f * cos_w0;
      b2_ = 1.0f + alpha;
      a0_ = 1.0f + alpha;
      a1_ = -2.0f * cos_w0;
      a2_ = 1.0f - alpha;
      break;

    case BiquadFilterType::Peaking:
      b0_ = 1.0f + alpha * A;
      b1_ = -2.0f * cos_w0;
      b2_ = 1.0f - alpha * A;
      a0_ = 1.0f + alpha / A;
      a1_ = -2.0f * cos_w0;
      a2_ = 1.0f - alpha / A;
      break;

    case BiquadFilterType::Lowshelf: {
      float sqrt_A = SDL_sqrtf(A);
      b0_ = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrt_A * alpha);
      b1_ = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_w0);
      b2_ = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrt_A * alpha);
      a0_ = (A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrt_A * alpha;
      a1_ = -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_w0);
      a2_ = (A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrt_A * alpha;
      break;
    }

    case BiquadFilterType::Highshelf: {
      float sqrt_A = SDL_sqrtf(A);
      b0_ = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrt_A * alpha);
      b1_ = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_w0);
      b2_ = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrt_A * alpha);
      a0_ = (A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrt_A * alpha;
      a1_ = 2.0f * ((A - 1.0f) - (A + 1.0f) * cos_w0);
      a2_ = (A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrt_A * alpha;
      break;
    }
  }
}

}  // namespace MySound
