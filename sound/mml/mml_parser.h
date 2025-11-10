#pragma once

#include <string>
#include "../types/note.h"
#include "../types/wave_type.h"
#include "../utilities/fixed_note_sequence.h"
#include "../utilities/music_utilities.h"

namespace MySound {

/**
 * @brief MML（Music Macro Language）パーサー
 *
 * MML文字列を解析してNoteDataのリストに変換します。
 * C++20のconstexprに対応しており、コンパイル時にMMLをパースできます。
 *
 * サポートするMML記法:
 * - c, d, e, f, g, a, b: ドレミファソラシ
 * - +, #: シャープ（半音上げる）
 * - -: フラット（半音下げる）
 * - 数字: 音符の長さ（1=全音符、2=2分音符、4=4分音符、8=8分音符など）
 * - o数字: オクターブ指定（o4など）
 * - r: 休符
 * - l数字: デフォルトの音符長を設定
 * - t数字: テンポ（BPM）を設定
 * - @数字: 音色を設定（0=サイン波、1=矩形波、2=ノコギリ波、3=ノイズ）
 * - v数字: ボリューム設定（v0〜v15で0.0〜1.0にマップ）
 * - .: 付点音符（音符の長さを1.5倍に）
 * - <: オクターブを1つ下げる
 * - >: オクターブを1つ上げる
 *
 * 例: "t120 o4 l4 cdefgab >c"
 */
class MMLParser {
 public:
  /**
   * @brief MML文字列を解析（constexpr対応）
   * @param mml MML文字列
   * @return 音符データのリスト（固定長配列）
   */
  static constexpr FixedNoteSequence parse(const std::string& mml) {
    FixedNoteSequence result;

    // デフォルト値
    float bpm = 120.0f;
    int default_length = 4;  // 4分音符
    int octave = 4;
    WaveType wave_type = WaveType::Sine;
    float volume = 1.0f;     // ボリューム（0.0〜1.0）

    size_t i = 0;
    while (i < mml.length()) {
      char c = constexpr_tolower(mml[i]);

      // 空白はスキップ
      if (constexpr_isspace(c)) {
        i++;
        continue;
      }

      // テンポ設定 (t120など)
      if (c == 't') {
        i++;
        int tempo = parseNumber(mml, i);
        if (tempo > 0) bpm = static_cast<float>(tempo);
        continue;
      }

      // デフォルト音符長設定 (l4など)
      if (c == 'l') {
        i++;
        int length = parseNumber(mml, i);
        if (length > 0) default_length = length;
        continue;
      }

      // オクターブ設定 (o4など)
      if (c == 'o') {
        i++;
        int oct = parseNumber(mml, i);
        if (oct >= 0 && oct <= 8) octave = oct;
        continue;
      }

      // 音色設定 (@0など)
      if (c == '@') {
        i++;
        int wave = parseNumber(mml, i);
        if (wave == 0) wave_type = WaveType::Sine;
        else if (wave == 1) wave_type = WaveType::Square;
        else if (wave == 2) wave_type = WaveType::Sawtooth;
        else if (wave == 3) wave_type = WaveType::Noise;
        continue;
      }

      // ボリューム設定 (v0〜v15など)
      if (c == 'v') {
        i++;
        int vol = parseNumber(mml, i);
        // v0〜v15を0.0〜1.0にマップ
        volume = static_cast<float>(vol) / 15.0f;
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;
        continue;
      }

      // オクターブ上げ
      if (c == '>') {
        if (octave < 8) octave++;
        i++;
        continue;
      }

      // オクターブ下げ
      if (c == '<') {
        if (octave > 0) octave--;
        i++;
        continue;
      }

      // 休符 (r4など)
      if (c == 'r') {
        i++;
        int length = default_length;
        if (i < mml.length() && constexpr_isdigit(mml[i])) {
          length = parseNumber(mml, i);
        }
        bool dotted = false;
        if (i < mml.length() && mml[i] == '.') {
          dotted = true;
          i++;
        }
        float duration = MusicUtil::noteDuration(bpm, length, dotted);
        result.push_back(NoteData(Note::C, 0, duration, true, wave_type, volume));
        continue;
      }

      // 音符 (c4, d+8など)
      if (c >= 'a' && c <= 'g') {
        Note note = charToNote(c);
        i++;

        // シャープ/フラット
        if (i < mml.length() && (mml[i] == '+' || mml[i] == '#')) {
          note = static_cast<Note>((static_cast<int>(note) + 1) % 12);
          i++;
        } else if (i < mml.length() && mml[i] == '-') {
          note = static_cast<Note>((static_cast<int>(note) + 11) % 12);
          i++;
        }

        // 音符の長さ
        int length = default_length;
        if (i < mml.length() && constexpr_isdigit(mml[i])) {
          length = parseNumber(mml, i);
        }

        // 付点音符
        bool dotted = false;
        if (i < mml.length() && mml[i] == '.') {
          dotted = true;
          i++;
        }

        float duration = MusicUtil::noteDuration(bpm, length, dotted);
        result.push_back(NoteData(note, octave, duration, false, wave_type, volume));
        continue;
      }

      // 未知の文字はスキップ
      i++;
    }

    return result;
  }

 private:
  /**
   * @brief constexpr版のtolower
   */
  static constexpr char constexpr_tolower(char c) {
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
  }

  /**
   * @brief constexpr版のisspace
   */
  static constexpr bool constexpr_isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
  }

  /**
   * @brief constexpr版のisdigit
   */
  static constexpr bool constexpr_isdigit(char c) {
    return c >= '0' && c <= '9';
  }

  /**
   * @brief 文字を音階に変換
   */
  static constexpr Note charToNote(char c) {
    switch (c) {
      case 'c': return Note::C;
      case 'd': return Note::D;
      case 'e': return Note::E;
      case 'f': return Note::F;
      case 'g': return Note::G;
      case 'a': return Note::A;
      case 'b': return Note::B;
      default: return Note::C;
    }
  }

  /**
   * @brief 文字列から数値を解析
   */
  static constexpr int parseNumber(const std::string& str, size_t& pos) {
    int result = 0;
    while (pos < str.length() && constexpr_isdigit(str[pos])) {
      result = result * 10 + (str[pos] - '0');
      pos++;
    }
    return result;
  }
};

/**
 * @brief MMLユーザー定義リテラル（constexpr対応）
 *
 * 使用例:
 *   constexpr auto notes = "t120 o4 cdefgab"_mml;  // コンパイル時評価（完全なconstexpr）
 *   auto notes = "t120 o4 cdefgab"_mml;            // 実行時評価も可能
 */
constexpr FixedNoteSequence operator""_mml(const char* str, size_t len) {
  return MMLParser::parse(std::string(str, len));
}

}  // namespace MySound
