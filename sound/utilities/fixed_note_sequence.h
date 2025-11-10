#pragma once

#include <array>
#include <span>
#include "../types/note.h"
#include "../sound_constants.h"

namespace MySound {

/**
 * @brief 固定サイズの音符シーケンス（constexpr対応）
 *
 * std::vectorの代わりにstd::arrayを使用することで、
 * constexprでの使用を可能にします。
 * MMLパーサーからの結果を格納するのに使用します。
 */
class FixedNoteSequence {
 public:
  /**
   * @brief デフォルトコンストラクタ
   */
  constexpr FixedNoteSequence() : size_(0), data_() {}

  /**
   * @brief std::spanからのコンストラクタ
   * @param notes 音符データのspan
   */
  constexpr FixedNoteSequence(std::span<const NoteData> notes)
      : size_(notes.size()), data_() {
    for (size_t i = 0; i < notes.size() && i < MAX_NOTE_SEQUENCE_SIZE; ++i) {
      data_[i] = notes[i];
    }
  }

  /**
   * @brief 音符を追加
   * @param note 音符データ
   */
  constexpr void push_back(const NoteData& note) {
    if (size_ < MAX_NOTE_SEQUENCE_SIZE) {
      data_[size_++] = note;
    }
  }

  /**
   * @brief 要素数を取得
   * @return 要素数
   */
  constexpr size_t size() const { return size_; }

  /**
   * @brief 最大サイズを取得
   * @return 最大サイズ
   */
  constexpr size_t capacity() const { return MAX_NOTE_SEQUENCE_SIZE; }

  /**
   * @brief 空かどうか
   * @return 空ならtrue
   */
  constexpr bool empty() const { return size_ == 0; }

  /**
   * @brief クリア
   */
  constexpr void clear() { size_ = 0; }

  /**
   * @brief 要素にアクセス
   * @param index インデックス
   * @return 音符データへの参照
   */
  constexpr const NoteData& operator[](size_t index) const {
    return data_[index];
  }

  /**
   * @brief 要素にアクセス（非const）
   * @param index インデックス
   * @return 音符データへの参照
   */
  constexpr NoteData& operator[](size_t index) {
    return data_[index];
  }

  /**
   * @brief イテレータの開始
   * @return 先頭要素へのポインタ
   */
  constexpr const NoteData* begin() const { return data_.data(); }

  /**
   * @brief イテレータの終端
   * @return 終端へのポインタ
   */
  constexpr const NoteData* end() const { return data_.data() + size_; }

  /**
   * @brief std::spanに変換
   * @return 音符データのspan
   */
  constexpr std::span<const NoteData> span() const {
    return std::span<const NoteData>(data_.data(), size_);
  }

 private:
  size_t size_;                                    // 実際の要素数
  std::array<NoteData, MAX_NOTE_SEQUENCE_SIZE> data_;  // 音符データ
};

}  // namespace MySound
