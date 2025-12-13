#ifndef MYGAME_BLINK_H_INCLUDED
#define MYGAME_BLINK_H_INCLUDED

#include <SDL3/SDL.h>

namespace MyGame {

// 前方宣言
class Entity;
class Component;

/**
 * @brief 点滅コンポーネント
 *
 * 周期的に表示フラグを切り替えます。
 * 点滅状態フラグ（blinking_flag_index）がセットされている場合のみ動作します。
 */
class Blink : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param interval_ms 点滅間隔（ミリ秒）
   * @param visible_flag_index 表示フラグのインデックス（デフォルト: 0）
   * @param blinking_flag_index 点滅状態フラグのインデックス（デフォルト: 2）
   */
  explicit Blink(Uint64 interval_ms = 500, size_t visible_flag_index = 0,
                 size_t blinking_flag_index = 2)
      : interval_(interval_ms), timer_(0), visible_flag_index_(visible_flag_index),
        blinking_flag_index_(blinking_flag_index) {}

  void update(Entity* entity, Uint64 delta_time) override;

  /**
   * @brief 点滅間隔を設定
   * @param interval_ms 点滅間隔（ミリ秒）
   */
  void setInterval(Uint64 interval_ms) { interval_ = interval_ms; }

  /**
   * @brief 点滅間隔を取得
   * @return 点滅間隔（ミリ秒）
   */
  Uint64 getInterval() const { return interval_; }

 private:
  Uint64 interval_;
  Uint64 timer_;
  size_t visible_flag_index_;
  size_t blinking_flag_index_;
};

}  // namespace MyGame

#endif  // MYGAME_BLINK_H_INCLUDED

// ========================================================================
// 実装部分（条件付きコンパイル）
// ========================================================================
#if defined(MYGAME_ENTITY_DEFINED) && !defined(MYGAME_BLINK_IMPL_INCLUDED)
#define MYGAME_BLINK_IMPL_INCLUDED

namespace MyGame {

// Blinkの実装
inline void Blink::update(Entity* entity, Uint64 delta_time) {
  if (entity->getStateFlag(blinking_flag_index_)) {
    timer_ += delta_time;
    if (timer_ > interval_) {
      entity->setStateFlag(visible_flag_index_,
                          !entity->getStateFlag(visible_flag_index_));
      timer_ = 0;
    }
  }
}

}  // namespace MyGame

#endif  // MYGAME_ENTITY_DEFINED && !MYGAME_BLINK_IMPL_INCLUDED
