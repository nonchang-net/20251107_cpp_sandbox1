#include "synthesizer.h"

// デバッグログの有効化フラグ（0 = 無効、1 = 有効）
#define SYNTH_DEBUG_LOG 0

#if SYNTH_DEBUG_LOG
  #define SYNTH_LOG(...) SDL_Log(__VA_ARGS__)
#else
  #define SYNTH_LOG(...) ((void)0)
#endif

namespace MySound {

SimpleSynthesizer::SimpleSynthesizer(int sample_rate)
    : oscillator_(std::make_unique<Oscillator>(WaveType::Sine, DEFAULT_FREQUENCY)),
      envelope_(std::make_unique<Envelope>()),
      effects_(),            // エフェクトチェーンは空で初期化
      stream_(nullptr),
      sample_rate_(sample_rate),
      current_sample_(0),
      master_volume_(DEFAULT_VOLUME),
      note_volume_(DEFAULT_VOLUME),
      is_playing_(false),
      gate_(false),
      note_on_time_(0.0f),
      note_off_time_(0.0f),
      note_duration_(0.0f),
      debug_first_samples_(false) {

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

SimpleSynthesizer::~SimpleSynthesizer() {
  if (stream_) {
    SDL_DestroyAudioStream(stream_);
  }
}

Oscillator& SimpleSynthesizer::getOscillator() {
  return *oscillator_;
}

Envelope& SimpleSynthesizer::getEnvelope() {
  return *envelope_;
}

void SimpleSynthesizer::setVolume(float volume) {
  master_volume_ = SDL_clamp(volume, 0.0f, 1.0f);
}

float SimpleSynthesizer::getVolume() const {
  return master_volume_;
}

void SimpleSynthesizer::noteOn(float frequency, float duration, float volume) {
  if (!stream_) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Audio stream not initialized");
    return;
  }

  oscillator_->setFrequency(frequency);
  current_sample_ = 0;  // サンプル位置をリセット
  note_on_time_ = 0.0f;
  note_off_time_ = 0.0f;
  note_duration_ = duration;
  note_volume_ = SDL_clamp(volume, 0.0f, 1.0f);
  gate_ = true;
  is_playing_ = true;
  debug_first_samples_ = true;  // デバッグログを有効化

  // エンベロープを開始
  envelope_->noteOn();

  SYNTH_LOG("NoteOn: %.2f Hz, duration: %.2f sec, volume: %.2f, stream=%p",
            frequency, duration, note_volume_, stream_);
}

void SimpleSynthesizer::noteOff() {
  if (gate_) {
    note_off_time_ = getCurrentTime();
    gate_ = false;

    // エンベロープをリリースフェーズに移行
    envelope_->noteOff();

    SYNTH_LOG("NoteOff at %.2f sec", note_off_time_);
  }
}

void SimpleSynthesizer::update() {
  if (!is_playing_) return;

  // 自動ノートオフ（duration指定がある場合）
  if (gate_ && note_duration_ > 0.0f) {
    float current_time = getCurrentTime();
    if (current_time >= note_duration_) {
      noteOff();
    }
  }

  // Releaseフェーズ完了チェック（エンベロープがIdleになったら停止）
  if (!gate_ && envelope_->getState() == Envelope::State::Idle) {
    is_playing_ = false;
    SYNTH_LOG("Sound finished (envelope reached idle)");
  }
}

int SimpleSynthesizer::getSampleRate() const {
  return sample_rate_;
}

void SimpleSynthesizer::addEffect(std::unique_ptr<AudioEffect> effect) {
  if (effect) {
    effects_.push_back(std::move(effect));
    SYNTH_LOG("Effect added to chain (total: %zu effects)", effects_.size());
  }
}

void SimpleSynthesizer::clearEffects() {
  effects_.clear();
  SYNTH_LOG("All effects cleared");
}

size_t SimpleSynthesizer::getEffectCount() const {
  return effects_.size();
}

void SDLCALL SimpleSynthesizer::audioCallback(void* userdata, SDL_AudioStream* stream,
                                               int additional_amount, int total_amount) {
  SimpleSynthesizer* synth = static_cast<SimpleSynthesizer*>(userdata);

  // 再生中でない場合は無音を出力
  if (!synth->is_playing_) {
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

void SimpleSynthesizer::generateSamples(float* samples, int num_samples) {
  for (int i = 0; i < num_samples; ++i) {
    // エンベロープの値を計算
    float envelope_value = envelope_->process(sample_rate_);

    // 位相を計算（0.0〜1.0）
    float frequency = oscillator_->getFrequency();
    float phase = SDL_fmodf(
        static_cast<float>(current_sample_) * frequency / sample_rate_, 1.0f);

    // 波形を生成
    float wave = oscillator_->generate(phase);

    // エンベロープとボリュームを適用
    // 最終ボリューム = 波形 × エンベロープ × ノートボリューム × マスターボリューム
    samples[i] = wave * envelope_value * note_volume_ * master_volume_;

    // エフェクトチェーンを適用（追加順に処理）
    for (auto& effect : effects_) {
      samples[i] = effect->process(samples[i]);
    }

    // クリッピング防止
    if (samples[i] > 1.0f) samples[i] = 1.0f;
    if (samples[i] < -1.0f) samples[i] = -1.0f;

    // 最初の数サンプルをログ出力（デバッグ用）
    if (debug_first_samples_ && i < 10) {
      SYNTH_LOG("Sample[%d]: env=%.4f, phase=%.4f, wave=%.4f, output=%.4f",
                i, envelope_value, phase, wave, samples[i]);
    }

    current_sample_++;
  }

  if (debug_first_samples_) {
    debug_first_samples_ = false;
    SYNTH_LOG("Generated %d samples, frequency=%.2f Hz", num_samples, oscillator_->getFrequency());
  }
}

float SimpleSynthesizer::getCurrentTime() const {
  return static_cast<float>(current_sample_) / sample_rate_;
}

}  // namespace MySound
