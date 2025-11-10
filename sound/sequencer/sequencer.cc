#include "sequencer.h"
#include "../utilities/music_utilities.h"
#include "../sound_constants.h"

namespace MySound {

Sequencer::Sequencer(SimpleSynthesizer* synthesizer, float bpm)
    : synthesizer_(synthesizer), bpm_(bpm), volume_(1.0f),
      current_note_index_(0), is_playing_(false), sequence_time_(0.0f),
      last_update_time_(0), loop_enabled_(false), loop_count_(-1),
      current_loop_(0), timer_id_(0),
      update_interval_ns_(DEFAULT_SEQUENCER_UPDATE_INTERVAL_NS) {
  // デフォルト: 15ms間隔（15,000,000ns）
}

Sequencer::~Sequencer() {
  stopTimer();
}

void Sequencer::setUpdateIntervalNS(Uint64 interval_ns) {
  update_interval_ns_ = interval_ns;
}

void Sequencer::setUpdateInterval(Uint32 interval_ms) {
  update_interval_ns_ = static_cast<Uint64>(interval_ms) * 1000000;  // ms to ns
}

void Sequencer::setVolume(float volume) {
  volume_ = SDL_clamp(volume, 0.0f, 1.0f);
}

void Sequencer::setLoop(bool enabled, int count) {
  loop_enabled_ = enabled;
  loop_count_ = count;
}

void Sequencer::clear() {
  sequence_.clear();
  current_note_index_ = 0;
  sequence_time_ = 0.0f;
}

void Sequencer::addNote(Note note, int octave, int note_division, bool dotted,
                        WaveType wave_type, float volume) {
  float duration = MusicUtil::noteDuration(bpm_, note_division, dotted);
  sequence_.emplace_back(note, octave, duration, false, wave_type, volume);
}

void Sequencer::addRest(int note_division, bool dotted) {
  float duration = MusicUtil::noteDuration(bpm_, note_division, dotted);
  // is_restフラグで休符を表現
  sequence_.emplace_back(Note::C, 0, duration, true, WaveType::Sine, 1.0f);
}

void Sequencer::setSequence(std::vector<NoteData>&& notes) {
  sequence_ = std::move(notes);
}

void Sequencer::setSequence(const std::vector<NoteData>& notes) {
  sequence_ = notes;
}

void Sequencer::setSequence(const FixedNoteSequence& notes) {
  sequence_.clear();
  sequence_.reserve(notes.size());
  for (const auto& note : notes) {
    sequence_.push_back(note);
  }
}

void Sequencer::setSequence(std::span<const NoteData> notes) {
  sequence_.clear();
  sequence_.reserve(notes.size());
  for (const auto& note : notes) {
    sequence_.push_back(note);
  }
}

void Sequencer::play() {
  if (sequence_.empty()) return;

  current_note_index_ = 0;
  sequence_time_ = 0.0f;
  current_loop_ = 0;
  is_playing_ = true;
  last_update_time_ = SDL_GetTicks();

  // 最初の音符を再生
  playCurrentNote();

  // 高精度タイマーを開始
  startTimer();
}

void Sequencer::stop() {
  is_playing_ = false;
  synthesizer_->noteOff();
  stopTimer();
}

void Sequencer::update() {
  // タイマーベースの更新に移行したため、このメソッドは空にする
  // 必要に応じて状態確認などの軽量処理のみ行う
}

Uint64 SDLCALL Sequencer::timerCallback(void* userdata, SDL_TimerID timerID, Uint64 interval) {
  Sequencer* sequencer = static_cast<Sequencer*>(userdata);
  if (sequencer) {
    sequencer->internalUpdate();
  }
  return interval;  // 同じ間隔で継続
}

void Sequencer::startTimer() {
  if (timer_id_ != 0) {
    stopTimer();  // 既存のタイマーを停止
  }
  timer_id_ = SDL_AddTimerNS(update_interval_ns_, timerCallback, this);
  if (timer_id_ == 0) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to create timer: %s", SDL_GetError());
  }
}

void Sequencer::stopTimer() {
  if (timer_id_ != 0) {
    SDL_RemoveTimer(timer_id_);
    timer_id_ = 0;
  }
}

void Sequencer::internalUpdate() {
  if (!is_playing_ || sequence_.empty()) return;

  // 経過時間を計算
  Uint64 current_time = SDL_GetTicks();
  float delta_time = (current_time - last_update_time_) / 1000.0f;
  last_update_time_ = current_time;

  sequence_time_ += delta_time;

  // 現在の音符が終了したか確認
  if (current_note_index_ < sequence_.size()) {
    const NoteData& current_note = sequence_[current_note_index_];
    if (sequence_time_ >= current_note.duration) {
      // 次の音符へ
      sequence_time_ -= current_note.duration;
      current_note_index_++;

      if (current_note_index_ < sequence_.size()) {
        playCurrentNote();
      } else {
        // シーケンス終了
        handleSequenceEnd();
      }
    }
  }
}

void Sequencer::handleSequenceEnd() {
  if (!loop_enabled_) {
    // ループが無効の場合は停止
    is_playing_ = false;
    return;
  }

  // ループ回数をチェック
  if (loop_count_ >= 0) {
    // 回数指定ループの場合
    current_loop_++;
    if (current_loop_ > loop_count_) {
      // 指定回数に達したので停止
      is_playing_ = false;
      return;
    }
  }
  // loop_count_ == -1の場合は無限ループ

  // シーケンスを最初から再生
  current_note_index_ = 0;
  sequence_time_ = 0.0f;
  playCurrentNote();
}

void Sequencer::playCurrentNote() {
  if (current_note_index_ >= sequence_.size()) return;

  const NoteData& note_data = sequence_[current_note_index_];

  // 休符の場合は何もしない
  if (note_data.is_rest) {
    synthesizer_->noteOff();
    return;
  }

  // 音色を設定
  synthesizer_->getOscillator().setWaveType(note_data.wave_type);

  // ボリュームを計算（シーケンサーボリューム × ノートボリューム）
  float final_volume = volume_ * note_data.volume;

  // 音符を再生
  float frequency = note_data.getFrequency();
  synthesizer_->noteOn(frequency, note_data.duration, final_volume);
}

}  // namespace MySound
