#include "audio_mixer.h"

// デバッグログの有効化フラグ（0 = 無効、1 = 有効）
#define MIXER_DEBUG_LOG 0

#if MIXER_DEBUG_LOG
  #define MIXER_LOG(...) SDL_Log(__VA_ARGS__)
#else
  #define MIXER_LOG(...) ((void)0)
#endif

namespace MySound {

AudioMixer::AudioMixer(int sample_rate, bool enable_stream)
    : synthesizers_(),
      effects_(),
      stream_(nullptr),
      sample_rate_(sample_rate),
      master_volume_(DEFAULT_VOLUME) {

  // ストリームありモードの場合のみ、オーディオストリームを初期化
  if (enable_stream) {
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
      SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "AudioMixer: Failed to open audio device: %s", SDL_GetError());
      return;
    }

    MIXER_LOG("AudioMixer: stream initialized (callback mode): %p, sample_rate=%d", stream_, sample_rate_);

    // オーディオデバイスを再開（デフォルトは一時停止状態）
    if (!SDL_ResumeAudioStreamDevice(stream_)) {
      SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "AudioMixer: Failed to resume audio device: %s", SDL_GetError());
    } else {
      MIXER_LOG("AudioMixer: device resumed successfully");
    }
  } else {
    MIXER_LOG("AudioMixer: created in stream-less mode (mixing only)");
  }
}

AudioMixer::~AudioMixer() {
  if (stream_) {
    SDL_DestroyAudioStream(stream_);
  }
}

void AudioMixer::addSynthesizer(SimpleSynthesizer* synth) {
  if (synth) {
    synthesizers_.push_back(synth);
    MIXER_LOG("AudioMixer: synthesizer added (total: %zu synthesizers)", synthesizers_.size());
  }
}

void AudioMixer::clearSynthesizers() {
  synthesizers_.clear();
  MIXER_LOG("AudioMixer: all synthesizers cleared");
}

size_t AudioMixer::getSynthesizerCount() const {
  return synthesizers_.size();
}

void AudioMixer::addEffect(std::unique_ptr<AudioEffect> effect) {
  if (effect) {
    effects_.push_back(std::move(effect));
    MIXER_LOG("AudioMixer: effect added to chain (total: %zu effects)", effects_.size());
  }
}

void AudioMixer::clearEffects() {
  effects_.clear();
  MIXER_LOG("AudioMixer: all effects cleared");
}

size_t AudioMixer::getEffectCount() const {
  return effects_.size();
}

void AudioMixer::setVolume(float volume) {
  master_volume_ = SDL_clamp(volume, 0.0f, 1.0f);
}

float AudioMixer::getVolume() const {
  return master_volume_;
}

int AudioMixer::getSampleRate() const {
  return sample_rate_;
}

void AudioMixer::generateSamples(float* samples, int num_samples) {
  mixSamples(samples, num_samples);
}

void SDLCALL AudioMixer::audioCallback(void* userdata, SDL_AudioStream* stream,
                                        int additional_amount, int total_amount) {
  AudioMixer* mixer = static_cast<AudioMixer*>(userdata);

  // 必要なサンプル数を計算
  int samples_needed = additional_amount / sizeof(float);
  if (samples_needed <= 0) return;

  // サンプルをミックス
  float* samples = new float[samples_needed];
  mixer->mixSamples(samples, samples_needed);

  // ストリームにデータを送信
  SDL_PutAudioStreamData(stream, samples, additional_amount);

  delete[] samples;
}

void AudioMixer::mixSamples(float* output, int num_samples) {
  // 出力バッファを初期化（無音）
  SDL_memset(output, 0, num_samples * sizeof(float));

  // シンセサイザーが登録されていない場合は無音を出力
  if (synthesizers_.empty()) {
    return;
  }

  // 各シンセサイザーからサンプルを取得してミックス
  float* temp_buffer = new float[num_samples];

  for (auto* synth : synthesizers_) {
    if (synth) {
      // シンセサイザーからサンプルを生成
      synth->generateSamples(temp_buffer, num_samples);

      // ミックス（加算）
      for (int i = 0; i < num_samples; ++i) {
        output[i] += temp_buffer[i];
      }
    }
  }

  delete[] temp_buffer;

  // マスターボリュームとエフェクトチェーンを適用
  for (int i = 0; i < num_samples; ++i) {
    // マスターボリュームを適用
    output[i] *= master_volume_;

    // エフェクトチェーンを適用（追加順に処理）
    for (auto& effect : effects_) {
      output[i] = effect->process(output[i]);
    }

    // クリッピング防止
    if (output[i] > 1.0f) output[i] = 1.0f;
    if (output[i] < -1.0f) output[i] = -1.0f;
  }
}

}  // namespace MySound
