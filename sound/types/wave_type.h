#pragma once

namespace MySound {

/**
 * @brief 波形の種類
 */
enum class WaveType {
  Sine,      // サイン波
  Square,    // 矩形波
  Sawtooth,  // ノコギリ波
  Noise      // ホワイトノイズ（LCG方式）
};

}  // namespace MySound
