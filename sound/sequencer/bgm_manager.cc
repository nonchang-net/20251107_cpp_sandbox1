#include "bgm_manager.h"
#include "../sound_constants.h"

namespace MySound {

BGMManager::BGMManager(int sample_rate)
    : stream_(nullptr),
      sample_rate_(sample_rate),
      last_update_time_(SDL_GetTicks()) {

  // オーディオストリームを初期化（コールバック方式）
  SDL_AudioSpec spec = {};     // ゼロ初期化
  spec.channels = 2;           // ステレオ（MultiTrackSequencerに合わせる）
  spec.format = SDL_AUDIO_F32; // 32ビット浮動小数点
  spec.freq = sample_rate_;

  // コールバック方式でオーディオストリームを開く
  stream_ = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
      &spec,
      audioCallback,  // コールバック関数
      this);          // ユーザーデータ（this）

  if (!stream_) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "BGMManager: Failed to open audio device: %s", SDL_GetError());
    return;
  }

  // オーディオデバイスを再開（デフォルトは一時停止状態）
  if (!SDL_ResumeAudioStreamDevice(stream_)) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "BGMManager: Failed to resume audio device: %s", SDL_GetError());
  }
}

BGMManager::~BGMManager() {
  // オーディオコールバックを停止するため、まずストリームを破棄
  if (stream_) {
    SDL_DestroyAudioStream(stream_);
    stream_ = nullptr;
  }

  // フェード状態の生ポインタをクリア（安全のため）
  fade_in_.bgm = nullptr;
  fade_out_.bgm = nullptr;

  // この後、デフォルトのデストラクタでbgm_map_が破棄される
}

void BGMManager::registerBGM(const std::string& id, std::unique_ptr<MultiTrackSequencer> bgm) {
  bgm_map_[id] = std::move(bgm);
}

MultiTrackSequencer* BGMManager::getBGM(const std::string& id) {
  auto it = bgm_map_.find(id);
  if (it != bgm_map_.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool BGMManager::play(const std::string& id) {
  auto* bgm = getBGM(id);
  if (!bgm) return false;

  // 現在再生中のBGMを停止
  if (!current_bgm_id_.empty() && current_bgm_id_ != id) {
    stop();
  }

  current_bgm_id_ = id;
  bgm->setMasterVolume(master_volume_);
  bgm->play();

  // フェード状態をリセット
  fade_in_.is_fading = false;
  fade_out_.is_fading = false;

  return true;
}

bool BGMManager::playWithCrossfade(const std::string& id, float fade_duration) {
  auto* new_bgm = getBGM(id);
  if (!new_bgm) return false;

  // 同じBGMの場合は何もしない
  if (current_bgm_id_ == id) {
    return true;
  }

  // フェードアウト設定（現在のBGM）
  if (!current_bgm_id_.empty()) {
    auto* current_bgm = getBGM(current_bgm_id_);
    if (current_bgm) {
      fade_out_.bgm = current_bgm;
      fade_out_.bgm_id = current_bgm_id_;
      fade_out_.current_volume = master_volume_;
      fade_out_.target_volume = 0.0f;
      fade_out_.fade_duration = fade_duration;
      fade_out_.elapsed_time = 0.0f;
      fade_out_.is_fading = true;
    }
  }

  // フェードイン設定（新しいBGM）
  fade_in_.bgm = new_bgm;
  fade_in_.bgm_id = id;
  fade_in_.current_volume = 0.0f;
  fade_in_.target_volume = master_volume_;
  fade_in_.fade_duration = fade_duration;
  fade_in_.elapsed_time = 0.0f;
  fade_in_.is_fading = true;

  // 新しいBGMを再生開始
  new_bgm->setMasterVolume(0.0f);  // 初期ボリュームは0
  new_bgm->play();

  current_bgm_id_ = id;

  return true;
}

void BGMManager::stop() {
  if (!current_bgm_id_.empty()) {
    auto* bgm = getBGM(current_bgm_id_);
    if (bgm) {
      bgm->stop();
    }
    current_bgm_id_.clear();
  }

  // フェード中のBGMも停止
  if (fade_in_.is_fading && fade_in_.bgm) {
    fade_in_.bgm->stop();
    fade_in_.is_fading = false;
  }
  if (fade_out_.is_fading && fade_out_.bgm) {
    fade_out_.bgm->stop();
    fade_out_.is_fading = false;
  }
}

void BGMManager::pause() {
  if (!current_bgm_id_.empty()) {
    auto* bgm = getBGM(current_bgm_id_);
    if (bgm) {
      bgm->pause();
    }
  }

  // フェード中のBGMも一時停止
  if (fade_in_.is_fading && fade_in_.bgm) {
    fade_in_.bgm->pause();
  }
  if (fade_out_.is_fading && fade_out_.bgm) {
    fade_out_.bgm->pause();
  }
}

void BGMManager::resume() {
  if (!current_bgm_id_.empty()) {
    auto* bgm = getBGM(current_bgm_id_);
    if (bgm) {
      bgm->resume();
    }
  }

  // フェード中のBGMも再開
  if (fade_in_.is_fading && fade_in_.bgm) {
    fade_in_.bgm->resume();
  }
  if (fade_out_.is_fading && fade_out_.bgm) {
    fade_out_.bgm->resume();
  }
}

void BGMManager::setMasterVolume(float volume) {
  master_volume_ = SDL_clamp(volume, 0.0f, 1.0f);

  // フェード中でないBGMのボリュームを更新
  if (!current_bgm_id_.empty() && !fade_in_.is_fading) {
    auto* bgm = getBGM(current_bgm_id_);
    if (bgm) {
      bgm->setMasterVolume(master_volume_);
    }
  }

  // フェードイン中の目標ボリュームを更新
  if (fade_in_.is_fading) {
    fade_in_.target_volume = master_volume_;
  }
}

bool BGMManager::isPlaying() const {
  if (current_bgm_id_.empty()) return false;
  auto it = bgm_map_.find(current_bgm_id_);
  if (it != bgm_map_.end()) {
    return it->second->isPlaying();
  }
  return false;
}

void BGMManager::update() {
  // デルタタイムを計算（秒）
  Uint64 current_time = SDL_GetTicks();
  float delta_time = (current_time - last_update_time_) / 1000.0f;
  last_update_time_ = current_time;

  // フェード処理を更新
  updateFade(fade_in_, delta_time);
  updateFade(fade_out_, delta_time);

  // 全てのBGMを更新
  for (auto& [id, bgm] : bgm_map_) {
    bgm->update();
  }
}

void BGMManager::updateFade(FadeState& state, float delta_time) {
  if (!state.is_fading || !state.bgm) return;

  state.elapsed_time += delta_time;

  // フェード進行度を計算（0.0〜1.0）
  float progress = SDL_clamp(state.elapsed_time / state.fade_duration, 0.0f, 1.0f);

  // 現在のボリュームを計算（線形補間）
  float start_volume = (state.target_volume == 0.0f) ? master_volume_ : 0.0f;
  float target_volume = state.target_volume;
  state.current_volume = start_volume + (target_volume - start_volume) * progress;

  // BGMのボリュームを設定
  state.bgm->setMasterVolume(state.current_volume);

  // フェード完了チェック
  if (progress >= 1.0f) {
    state.is_fading = false;

    // フェードアウト完了時は停止
    if (state.target_volume == 0.0f) {
      state.bgm->stop();
      state.bgm = nullptr;
    }
  }
}

void SDLCALL BGMManager::audioCallback(void* userdata, SDL_AudioStream* stream,
                                        int additional_amount, int total_amount) {
  BGMManager* manager = static_cast<BGMManager*>(userdata);

  // 必要なサンプル数を計算
  int samples_needed = additional_amount / sizeof(float);
  if (samples_needed <= 0) return;

  // サンプルをミックス
  float* samples = new float[samples_needed];
  manager->mixSamples(samples, samples_needed);

  // ストリームにデータを送信
  SDL_PutAudioStreamData(stream, samples, additional_amount);

  delete[] samples;
}

void BGMManager::mixSamples(float* output, int num_samples) {
  // 出力バッファを初期化（無音）
  SDL_memset(output, 0, num_samples * sizeof(float));

  float* temp_buffer = new float[num_samples];

  // フェードイン中のBGMをミックス
  if (fade_in_.is_fading && fade_in_.bgm) {
    fade_in_.bgm->generateSamples(temp_buffer, num_samples);
    for (int i = 0; i < num_samples; ++i) {
      output[i] += temp_buffer[i];
    }
  }

  // フェードアウト中のBGMをミックス
  if (fade_out_.is_fading && fade_out_.bgm) {
    fade_out_.bgm->generateSamples(temp_buffer, num_samples);
    for (int i = 0; i < num_samples; ++i) {
      output[i] += temp_buffer[i];
    }
  }

  // フェード中でない現在のBGMをミックス
  if (!current_bgm_id_.empty() && !fade_in_.is_fading && !fade_out_.is_fading) {
    auto* bgm = getBGM(current_bgm_id_);
    if (bgm) {
      bgm->generateSamples(temp_buffer, num_samples);
      for (int i = 0; i < num_samples; ++i) {
        output[i] += temp_buffer[i];
      }
    }
  }

  delete[] temp_buffer;

  // マスターボリュームとクリッピング防止
  for (int i = 0; i < num_samples; ++i) {
    // クリッピング防止
    if (output[i] > 1.0f) output[i] = 1.0f;
    if (output[i] < -1.0f) output[i] = -1.0f;
  }
}

}  // namespace MySound
