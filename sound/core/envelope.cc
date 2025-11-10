#include "envelope.h"

namespace MySound {

Envelope::Envelope()
    : attack_time_(DEFAULT_ATTACK_TIME),
      decay_time_(DEFAULT_DECAY_TIME),
      sustain_level_(DEFAULT_SUSTAIN_LEVEL),
      release_time_(DEFAULT_RELEASE_TIME),
      state_(State::Idle),
      current_level_(0.0f),
      release_level_(0.0f) {}

void Envelope::setADSR(float attack_time, float decay_time, float sustain_level, float release_time) {
  attack_time_ = attack_time;
  decay_time_ = decay_time;
  sustain_level_ = SDL_clamp(sustain_level, 0.0f, 1.0f);
  release_time_ = release_time;
}

void Envelope::setAttackTime(float time) {
  attack_time_ = time;
}

void Envelope::setDecayTime(float time) {
  decay_time_ = time;
}

void Envelope::setSustainLevel(float level) {
  sustain_level_ = SDL_clamp(level, 0.0f, 1.0f);
}

void Envelope::setReleaseTime(float time) {
  release_time_ = time;
}

void Envelope::noteOn() {
  state_ = State::Attack;
  current_level_ = 0.0f;
}

void Envelope::noteOff() {
  if (state_ != State::Idle) {
    state_ = State::Release;
    release_level_ = current_level_;
  }
}

float Envelope::process(int sample_rate) {
  const float dt = 1.0f / sample_rate;  // 1サンプルあたりの時間

  switch (state_) {
    case State::Idle:
      return 0.0f;

    case State::Attack:
      // アタックフェーズ: 0.0 → 1.0
      if (attack_time_ > 0.0f) {
        current_level_ += dt / attack_time_;
        if (current_level_ >= 1.0f) {
          current_level_ = 1.0f;
          state_ = State::Decay;
        }
      } else {
        current_level_ = 1.0f;
        state_ = State::Decay;
      }
      return current_level_;

    case State::Decay:
      // ディケイフェーズ: 1.0 → sustain_level_
      if (decay_time_ > 0.0f) {
        current_level_ -= dt * (1.0f - sustain_level_) / decay_time_;
        if (current_level_ <= sustain_level_) {
          current_level_ = sustain_level_;
          state_ = State::Sustain;
        }
      } else {
        current_level_ = sustain_level_;
        state_ = State::Sustain;
      }
      return current_level_;

    case State::Sustain:
      // サステインフェーズ: sustain_level_を維持
      return sustain_level_;

    case State::Release:
      // リリースフェーズ: release_level_ → 0.0
      if (release_time_ > 0.0f) {
        current_level_ = release_level_ * (1.0f - (dt / release_time_));
        release_level_ = current_level_;
        if (current_level_ <= 0.0f) {
          current_level_ = 0.0f;
          state_ = State::Idle;
        }
      } else {
        current_level_ = 0.0f;
        state_ = State::Idle;
      }
      return current_level_;

    default:
      return 0.0f;
  }
}

Envelope::State Envelope::getState() const {
  return state_;
}

}  // namespace MySound
