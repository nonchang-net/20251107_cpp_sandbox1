#pragma once

namespace MyGame {

// 前方宣言
class Entity;
class Component;

/**
 * @brief 4方向の向きを表すenum
 */
enum class Direction {
  Down = 0,   // 下向き
  Up = 1,     // 上向き
  Right = 2,  // 右向き
  Left = 3    // 左向き
};

/**
 * @brief エンティティの向き（方向）を保持するコンポーネント
 *
 * キャラクターの向きを管理します。
 * 向きを持たないエンティティには追加不要です。
 */
class DirectionComponent : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param direction 初期の向き
   */
  explicit DirectionComponent(Direction direction = Direction::Down)
      : direction_(direction) {}

  /**
   * @brief 向きを設定
   * @param direction 新しい向き
   */
  void setDirection(Direction direction) { direction_ = direction; }

  /**
   * @brief 向きを取得
   * @return 現在の向き
   */
  Direction getDirection() const { return direction_; }

 private:
  Direction direction_;
};

}  // namespace MyGame
