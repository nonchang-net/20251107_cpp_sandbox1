#pragma once

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
