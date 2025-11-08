#pragma once

#include <SDL3/SDL.h>

#include <functional>
#include <string>

#include "../entity_manager.h"

namespace MyGame::Entities {

/**
 * @brief 汎用的なテキストエンティティ
 *
 * テキストを画面に描画します。
 * 静的なテキストと動的に更新されるテキストの両方をサポートします。
 */
class TextEntity : public Entity {
 public:
  /**
   * @brief 静的テキスト用コンストラクタ
   * @param layer レイヤー番号
   * @param x X座標
   * @param y Y座標
   * @param text 表示するテキスト
   */
  TextEntity(int layer, float x, float y, const std::string& text)
      : Entity(layer), x_(x), y_(y), text_(text), color_{255, 255, 255, 255},
        text_provider_(nullptr) {}

  /**
   * @brief 動的テキスト用コンストラクタ
   * @param layer レイヤー番号
   * @param x X座標
   * @param y Y座標
   * @param text_provider テキストを返す関数
   */
  TextEntity(
      int layer, float x, float y,
      std::function<std::string()> text_provider)
      : Entity(layer), x_(x), y_(y), color_{255, 255, 255, 255},
        text_provider_(text_provider) {}

  void update(Uint64 delta_time) override {
    // 動的テキストの場合、毎フレーム更新
    if (text_provider_) {
      text_ = text_provider_();
    }
  }

  void render(SDL_Renderer* renderer) override {
    SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, color_.a);
    SDL_RenderDebugText(renderer, x_, y_, text_.c_str());
  }

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
   * @brief 位置を設定
   * @param x X座標
   * @param y Y座標
   */
  void setPosition(float x, float y) {
    x_ = x;
    y_ = y;
  }

  /**
   * @brief 位置を取得
   * @return {x, y}
   */
  std::pair<float, float> getPosition() const { return {x_, y_}; }

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
  float x_, y_;
  std::string text_;
  SDL_Color color_;
  std::function<std::string()> text_provider_;
};

}  // namespace MyGame::Entities
