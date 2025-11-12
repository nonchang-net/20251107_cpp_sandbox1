#include "audio_mixer.h"
#include <cmath>

// デバッグログの有効化フラグ（0 = 無効、1 = 有効）
#define MIXER_DEBUG_LOG 0

#if MIXER_DEBUG_LOG
  #define MIXER_LOG(...) SDL_Log(__VA_ARGS__)
#else
  #define MIXER_LOG(...) ((void)0)
#endif

namespace MySound {

AudioMixer::AudioMixer(int sample_rate, int num_output_channels, bool enable_stream)
    : synthesizers_(),
      effects_(),
      stream_(nullptr),
      sample_rate_(sample_rate),
      num_output_channels_(num_output_channels),
      master_volume_(DEFAULT_VOLUME),
      send_levels_() {

  // ストリームありモードの場合のみ、オーディオストリームを初期化
  if (enable_stream) {
    // オーディオストリームを初期化（コールバック方式）
    SDL_AudioSpec spec = {};  // ゼロ初期化
    spec.channels = num_output_channels_;  // 出力チャンネル数（可変）
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
  // オーディオコールバックを停止するため、まずストリームを破棄
  if (stream_) {
    SDL_DestroyAudioStream(stream_);
    stream_ = nullptr;
  }

  // シンセサイザーへの生ポインタをクリア（安全のため）
  synthesizers_.clear();
}

void AudioMixer::addSynthesizer(SimpleSynthesizer* synth) {
  if (synth) {
    synthesizers_.push_back(synth);

    // デフォルトのセンドレベルを設定
    std::vector<float> default_levels(num_output_channels_);

    if (num_output_channels_ == 1) {
      // モノラル出力の場合: フルレベル
      default_levels[0] = 1.0f;
    } else if (num_output_channels_ == 2) {
      // ステレオ出力の場合: 中央定位（L=0.707, R=0.707で等パワーパン）
      // √2/2 ≈ 0.707 で、L^2 + R^2 = 1.0 となり、パワーが保存される
      default_levels[0] = 0.707f;  // 左チャンネル
      default_levels[1] = 0.707f;  // 右チャンネル
    } else {
      // 3チャンネル以上の場合: 全チャンネルに均等配分
      float level = 1.0f / std::sqrt(static_cast<float>(num_output_channels_));
      for (int i = 0; i < num_output_channels_; ++i) {
        default_levels[i] = level;
      }
    }

    send_levels_.push_back(default_levels);

    MIXER_LOG("AudioMixer: synthesizer added (total: %zu synthesizers)", synthesizers_.size());
  }
}

void AudioMixer::clearSynthesizers() {
  synthesizers_.clear();
  send_levels_.clear();
  MIXER_LOG("AudioMixer: all synthesizers cleared");
}

size_t AudioMixer::getSynthesizerCount() const {
  return synthesizers_.size();
}

void AudioMixer::setSendLevel(size_t synth_index, size_t output_channel, float level) {
  if (synth_index >= send_levels_.size()) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO,
                 "AudioMixer::setSendLevel: invalid synth_index %zu (max: %zu)",
                 synth_index, send_levels_.size() - 1);
    return;
  }
  if (output_channel >= static_cast<size_t>(num_output_channels_)) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO,
                 "AudioMixer::setSendLevel: invalid output_channel %zu (max: %d)",
                 output_channel, num_output_channels_ - 1);
    return;
  }

  send_levels_[synth_index][output_channel] = SDL_clamp(level, 0.0f, 1.0f);
  MIXER_LOG("AudioMixer: send level set: synth[%zu] -> out[%zu] = %.3f",
            synth_index, output_channel, send_levels_[synth_index][output_channel]);
}

float AudioMixer::getSendLevel(size_t synth_index, size_t output_channel) const {
  if (synth_index >= send_levels_.size()) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO,
                 "AudioMixer::getSendLevel: invalid synth_index %zu",
                 synth_index);
    return 0.0f;
  }
  if (output_channel >= static_cast<size_t>(num_output_channels_)) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO,
                 "AudioMixer::getSendLevel: invalid output_channel %zu",
                 output_channel);
    return 0.0f;
  }

  return send_levels_[synth_index][output_channel];
}

void AudioMixer::setPan(size_t synth_index, float pan) {
  if (num_output_channels_ != 2) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO,
                 "AudioMixer::setPan: pan is only supported for 2-channel output (current: %d)",
                 num_output_channels_);
    return;
  }

  // パン値をクランプ（-1.0〜1.0）
  pan = SDL_clamp(pan, -1.0f, 1.0f);

  // 等パワーパン則を使用
  // pan = -1.0 (左)  -> L=1.0, R=0.0
  // pan =  0.0 (中央) -> L=0.707, R=0.707
  // pan =  1.0 (右)  -> L=0.0, R=1.0
  float angle = (pan + 1.0f) * 0.25f * M_PI;  // -1.0〜1.0 を 0〜π/2 にマッピング
  float left_level = std::cos(angle);
  float right_level = std::sin(angle);

  setSendLevel(synth_index, 0, left_level);
  setSendLevel(synth_index, 1, right_level);

  MIXER_LOG("AudioMixer: pan set: synth[%zu] pan=%.3f -> L=%.3f, R=%.3f",
            synth_index, pan, left_level, right_level);
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

int AudioMixer::getNumOutputChannels() const {
  return num_output_channels_;
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

  // フレーム数を計算（num_samplesはインターリーブされたサンプル数）
  int num_frames = num_samples / num_output_channels_;

  // 各シンセサイザーからサンプルを取得してミックス
  float* temp_buffer = new float[num_frames];

  for (size_t synth_idx = 0; synth_idx < synthesizers_.size(); ++synth_idx) {
    auto* synth = synthesizers_[synth_idx];
    if (synth) {
      // シンセサイザーからモノラルサンプルを生成
      synth->generateSamples(temp_buffer, num_frames);

      // センドレベルに応じて各出力チャンネルに配分
      const auto& send_levels = send_levels_[synth_idx];
      for (int frame = 0; frame < num_frames; ++frame) {
        float sample = temp_buffer[frame];
        for (int ch = 0; ch < num_output_channels_; ++ch) {
          output[frame * num_output_channels_ + ch] += sample * send_levels[ch];
        }
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
