#include "multi_track_sequencer.h"
#include "../sound_constants.h"

namespace MySound {

MultiTrackSequencer::MultiTrackSequencer(size_t track_count, int sample_rate, float bpm)
    : track_count_(track_count),
      bpm_(bpm),
      master_volume_(1.0f),
      is_paused_(false),
      mixer_(std::make_unique<AudioMixer>(sample_rate)) {

  // トラック数分のシンセサイザーとシーケンサーを生成
  // 注: シンセサイザーはストリームなしモード（enable_stream=false）で作成
  //     オーディオ出力はミキサーが一括管理
  for (size_t i = 0; i < track_count; ++i) {
    auto synth = std::make_unique<SimpleSynthesizer>(sample_rate, false);  // ストリームなし
    auto seq = std::make_unique<Sequencer>(synth.get(), bpm);

    // ミキサーにシンセサイザーを登録
    mixer_->addSynthesizer(synth.get());

    synthesizers_.push_back(std::move(synth));
    sequencers_.push_back(std::move(seq));
  }

  // ミキサーのマスターボリュームを設定
  mixer_->setVolume(master_volume_);
}

SimpleSynthesizer* MultiTrackSequencer::getSynthesizer(size_t track_index) {
  if (track_index >= track_count_) return nullptr;
  return synthesizers_[track_index].get();
}

Sequencer* MultiTrackSequencer::getSequencer(size_t track_index) {
  if (track_index >= track_count_) return nullptr;
  return sequencers_[track_index].get();
}

void MultiTrackSequencer::setTrackSequence(size_t track_index, const FixedNoteSequence& notes) {
  if (track_index >= track_count_) return;
  sequencers_[track_index]->clear();
  sequencers_[track_index]->setSequence(notes);
}

void MultiTrackSequencer::setTrackSequence(size_t track_index, const std::vector<NoteData>& notes) {
  if (track_index >= track_count_) return;
  sequencers_[track_index]->clear();
  sequencers_[track_index]->setSequence(notes);
}

void MultiTrackSequencer::setMasterVolume(float volume) {
  master_volume_ = SDL_clamp(volume, 0.0f, 1.0f);
  // ミキサーのマスターボリュームを設定
  mixer_->setVolume(master_volume_);
}

void MultiTrackSequencer::setLoop(bool enabled, int count) {
  for (auto& seq : sequencers_) {
    seq->setLoop(enabled, count);
  }
}

void MultiTrackSequencer::setUpdateIntervalNS(Uint64 interval_ns) {
  for (auto& seq : sequencers_) {
    seq->setUpdateIntervalNS(interval_ns);
  }
}

void MultiTrackSequencer::setUpdateInterval(Uint32 interval_ms) {
  for (auto& seq : sequencers_) {
    seq->setUpdateInterval(interval_ms);
  }
}

void MultiTrackSequencer::play() {
  is_paused_ = false;
  for (auto& seq : sequencers_) {
    seq->play();
  }
}

void MultiTrackSequencer::stop() {
  is_paused_ = false;
  for (auto& seq : sequencers_) {
    seq->stop();
  }
}

void MultiTrackSequencer::pause() {
  if (!is_paused_) {
    is_paused_ = true;
    for (auto& seq : sequencers_) {
      seq->stop();  // 現在はstopで代用（将来的にpause実装）
    }
  }
}

void MultiTrackSequencer::resume() {
  if (is_paused_) {
    is_paused_ = false;
    for (auto& seq : sequencers_) {
      seq->play();  // 現在はplayで代用（将来的にresume実装）
    }
  }
}

bool MultiTrackSequencer::isPlaying() const {
  for (const auto& seq : sequencers_) {
    if (seq->isPlaying()) return true;
  }
  return false;
}

void MultiTrackSequencer::update() {
  for (auto& synth : synthesizers_) {
    synth->update();
  }
  for (auto& seq : sequencers_) {
    seq->update();
  }
}

void MultiTrackSequencer::addMasterEffect(std::unique_ptr<AudioEffect> effect) {
  mixer_->addEffect(std::move(effect));
}

void MultiTrackSequencer::clearMasterEffects() {
  mixer_->clearEffects();
}

size_t MultiTrackSequencer::getMasterEffectCount() const {
  return mixer_->getEffectCount();
}

AudioMixer* MultiTrackSequencer::getMixer() {
  return mixer_.get();
}

}  // namespace MySound
