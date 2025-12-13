#pragma once

#include "blink.h"

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
