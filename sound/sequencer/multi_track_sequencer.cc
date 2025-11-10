#include "multi_track_sequencer.h"
#include "../sound_constants.h"

namespace MySound {

MultiTrackSequencer::MultiTrackSequencer(size_t track_count, int sample_rate, float bpm)
    : track_count_(track_count), bpm_(bpm), master_volume_(1.0f), is_paused_(false) {
  // トラック数分のシンセサイザーとシーケンサーを生成
  for (size_t i = 0; i < track_count; ++i) {
    auto synth = std::make_unique<SimpleSynthesizer>(sample_rate);
    auto seq = std::make_unique<Sequencer>(synth.get(), bpm);

    synthesizers_.push_back(std::move(synth));
    sequencers_.push_back(std::move(seq));
  }
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
  for (auto& synth : synthesizers_) {
    synth->setVolume(master_volume_);
  }
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

}  // namespace MySound
