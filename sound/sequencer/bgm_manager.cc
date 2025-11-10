#include "bgm_manager.h"
#include <SDL3/SDL.h>

namespace MySound {

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
  bgm->play();
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
}

void BGMManager::pause() {
  if (!current_bgm_id_.empty()) {
    auto* bgm = getBGM(current_bgm_id_);
    if (bgm) {
      bgm->pause();
    }
  }
}

void BGMManager::resume() {
  if (!current_bgm_id_.empty()) {
    auto* bgm = getBGM(current_bgm_id_);
    if (bgm) {
      bgm->resume();
    }
  }
}

void BGMManager::setMasterVolume(float volume) {
  master_volume_ = SDL_clamp(volume, 0.0f, 1.0f);
  for (auto& [id, bgm] : bgm_map_) {
    bgm->setMasterVolume(master_volume_);
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
  if (!current_bgm_id_.empty()) {
    auto* bgm = getBGM(current_bgm_id_);
    if (bgm) {
      bgm->update();
    }
  }
}

}  // namespace MySound
