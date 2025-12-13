#ifndef MYGAME_TEXT_RENDERER_H_INCLUDED
#define MYGAME_TEXT_RENDERER_H_INCLUDED

#include <SDL3/SDL.h>

#include <functional>
#include <string>

namespace MyGame {

// 前方宣言
class Entity;
class Component;
class UIAnchorComponent;

/**
 * @brief テキストを描画するコンポーネント
 *
 * 静的テキストと動的テキスト（関数）の両方をサポートします。
 * UIAnchorComponentと組み合わせることで、UI要素として使用できます。
 */
class TextRenderer : public Component {
 public:
  /**
   * @brief 静的テキスト用コンストラクタ
   * @param text 表示するテキスト
   * @param color 色
   */
  TextRenderer(const std::string& text, SDL_Color color = {255, 255, 255, 255})
      : text_(text), color_(color), text_provider_(nullptr) {}

  /**
   * @brief 動的テキスト用コンストラクタ
   * @param text_provider テキストを返す関数
   * @param color 色
   */
  TextRenderer(std::function<std::string()> text_provider,
               SDL_Color color = {255, 255, 255, 255})
      : color_(color), text_provider_(text_provider) {}

  void update(Entity* entity, Uint64 delta_time) override;
  void render(Entity* entity, SDL_Renderer* renderer) override;

  /**
   * @brief テキストを設定（静的テキスト）
   * @param text 新しいテキスト
   */
  void setText(const std::string& text) { text_ = text; }

  /**
   * @brief テキストを取得
   * @return 現在のテキスト
   */
  const std::string& getText() const { return text_; }

  /**
   * @brief 色を設定
   * @param color 色
   */
  void setColor(SDL_Color color) { color_ = color; }

  /**
   * @brief 色を取得
   * @return 色
   */
  SDL_Color getColor() const { return color_; }

  /**
   * @brief テキストプロバイダーを設定（動的テキスト）
   * @param provider テキストを返す関数
   */
  void setTextProvider(std::function<std::string()> provider) {
    text_provider_ = provider;
  }

 private:
  std::string text_;                           // テキスト内容
  SDL_Color color_;                            // 色
  std::function<std::string()> text_provider_; // 動的テキスト生成関数
};

}  // namespace MyGame

#endif  // MYGAME_TEXT_RENDERER_H_INCLUDED

// ========================================================================
// 実装部分（条件付きコンパイル）
// ========================================================================
#if defined(MYGAME_ENTITY_DEFINED) && !defined(MYGAME_TEXT_RENDERER_IMPL_INCLUDED)
#define MYGAME_TEXT_RENDERER_IMPL_INCLUDED

namespace MyGame {

// TextRendererの実装
inline void TextRenderer::update(Entity* entity, Uint64 delta_time) {
  // 動的テキストの場合、毎フレーム更新
  if (text_provider_) {
    text_ = text_provider_();
  }
}

inline void TextRenderer::render(Entity* entity, SDL_Renderer* renderer) {
  float screen_x, screen_y;

  // UIAnchorコンポーネントがあるかチェック
  if (auto* ui_anchor = entity->getComponent<UIAnchorComponent>()) {
    // UI要素として扱う：カメラの影響を受けず、画面座標で描画
    auto* camera = entity->getRenderCamera();
    float viewport_width = 640.0f;   // デフォルト値
    float viewport_height = 480.0f;  // デフォルト値

    // カメラからビューポートサイズを取得
    if (camera) {
      // Camera2Dからビューポートサイズを取得（将来的に実装）
      // 現在はデフォルト値を使用
    }

    // アンカー座標を計算
    auto [anchor_x, anchor_y] = ui_anchor->calculateAnchorPosition(
        viewport_width, viewport_height);

    // Locatorのオフセットを取得
    auto [offset_x, offset_y] = entity->getLocalPosition();

    // 画面座標 = アンカー座標 + オフセット
    screen_x = anchor_x + offset_x;
    screen_y = anchor_y + offset_y;
  } else {
    // ゲーム要素として扱う：ワールド座標 + カメラ変換
    auto [world_x, world_y] = entity->getWorldPosition();

    // カメラを使用してワールド座標から画面座標に変換
    if (auto* camera = entity->getRenderCamera()) {
      auto [sx, sy] = camera->worldToScreen(world_x, world_y);
      screen_x = sx;
      screen_y = sy;
    } else {
      screen_x = world_x;
      screen_y = world_y;
    }
  }

  // テキストを描画
  SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, color_.a);
  SDL_RenderDebugText(renderer, screen_x, screen_y, text_.c_str());
}

}  // namespace MyGame

#endif  // MYGAME_ENTITY_DEFINED && !MYGAME_TEXT_RENDERER_IMPL_INCLUDED
