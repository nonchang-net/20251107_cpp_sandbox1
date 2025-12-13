#pragma once

#include <utility>

namespace MyGame {

// 前方宣言
class Entity;
class Component;

/**
 * @brief UIアンカーの基準点
 */
enum class UIAnchor {
  TopLeft,      // 左上
  TopRight,     // 右上
  BottomLeft,   // 左下
  BottomRight,  // 右下
  Center        // 中央
};

/**
 * @brief UIアンカーコンポーネント
 *
 * UI要素を画面上の特定位置にアンカーします。
 * このコンポーネントがある場合、Locatorの座標はアンカーからのオフセットとして扱われます。
 * カメラの影響を受けず、常に画面座標で描画されます。
 */
class UIAnchorComponent : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param anchor アンカー位置
   */
  explicit UIAnchorComponent(UIAnchor anchor = UIAnchor::TopLeft)
      : anchor_(anchor) {}

  /**
   * @brief アンカーを設定
   * @param anchor アンカー位置
   */
  void setAnchor(UIAnchor anchor) { anchor_ = anchor; }

  /**
   * @brief アンカーを取得
   * @return アンカー位置
   */
  UIAnchor getAnchor() const { return anchor_; }

  /**
   * @brief アンカー座標を計算
   * @param viewport_width ビューポート幅
   * @param viewport_height ビューポート高さ
   * @return アンカーの画面座標 {x, y}
   */
  std::pair<float, float> calculateAnchorPosition(
      float viewport_width, float viewport_height) const {
    switch (anchor_) {
      case UIAnchor::TopLeft:
        return {0.0f, 0.0f};
      case UIAnchor::TopRight:
        return {viewport_width, 0.0f};
      case UIAnchor::BottomLeft:
        return {0.0f, viewport_height};
      case UIAnchor::BottomRight:
        return {viewport_width, viewport_height};
      case UIAnchor::Center:
        return {viewport_width / 2.0f, viewport_height / 2.0f};
      default:
        return {0.0f, 0.0f};
    }
  }

 private:
  UIAnchor anchor_;
};

}  // namespace MyGame
