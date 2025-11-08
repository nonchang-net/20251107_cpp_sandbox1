#pragma once

#include <SDL3/SDL.h>

#include "../entity_manager.h"

namespace MyGame::Entities {

/**
 * @brief 汎用的な矩形エンティティ
 *
 * 色、位置、サイズを持つ矩形を描画します。
 * 速度を設定することで移動させることができます。
 */
class RectEntity : public Entity {
 public:
  /**
   * @brief コンストラクタ
   * @param layer レイヤー番号
   * @param x X座標
   * @param y Y座標
   * @param w 幅
   * @param h 高さ
   * @param color 色
   */
  RectEntity(int layer, float x, float y, float w, float h, SDL_Color color)
      : Entity(layer), rect_{x, y, w, h}, color_(color), velocity_x_(0.0f),
        velocity_y_(0.0f) {}

  void update(Uint64 delta_time) override {
    // 移動処理
    rect_.x += velocity_x_;
    rect_.y += velocity_y_;
  }

  void render(SDL_Renderer* renderer) override {
    SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, color_.a);
    SDL_RenderFillRect(renderer, &rect_);
  }

  /**
   * @brief 速度を設定
   * @param vx X方向の速度
   * @param vy Y方向の速度
   */
  void setVelocity(float vx, float vy) {
    velocity_x_ = vx;
    velocity_y_ = vy;
  }

  /**
   * @brief 速度を取得
   * @return {vx, vy}
   */
  std::pair<float, float> getVelocity() const {
    return {velocity_x_, velocity_y_};
  }

  /**
   * @brief 位置を設定
   * @param x X座標
   * @param y Y座標
   */
  void setPosition(float x, float y) {
    rect_.x = x;
    rect_.y = y;
  }

  /**
   * @brief 位置を取得
   * @return {x, y}
   */
  std::pair<float, float> getPosition() const { return {rect_.x, rect_.y}; }

  /**
   * @brief サイズを設定
   * @param w 幅
   * @param h 高さ
   */
  void setSize(float w, float h) {
    rect_.w = w;
    rect_.h = h;
  }

  /**
   * @brief サイズを取得
   * @return {w, h}
   */
  std::pair<float, float> getSize() const { return {rect_.w, rect_.h}; }

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
   * @brief 矩形を取得
   * @return SDL_FRect
   */
  const SDL_FRect& getRect() const { return rect_; }

 private:
  SDL_FRect rect_;
  SDL_Color color_;
  float velocity_x_;
  float velocity_y_;
};

}  // namespace MyGame::Entities
