#pragma once

#include <SDL3/SDL.h>

#include <cmath>

#include "../entity_manager.h"

namespace MyGame::Entities {

/**
 * @brief 回転する矩形エンティティ
 *
 * 色、位置、サイズ、回転角度を持つ矩形を描画します。
 * 回転速度を設定することで回転させることができます。
 */
class RotateRectEntity : public Entity {
 public:
  /**
   * @brief コンストラクタ
   * @param layer レイヤー番号
   * @param x X座標（中心）
   * @param y Y座標（中心）
   * @param w 幅
   * @param h 高さ
   * @param color 色
   * @param angle 初期回転角度（度数法）
   */
  RotateRectEntity(
      int layer, float x, float y, float w, float h, SDL_Color color,
      float angle = 0.0f)
      : Entity(layer), center_x_(x), center_y_(y), width_(w), height_(h),
        color_(color), angle_(angle), angular_velocity_(0.0f), velocity_x_(0.0f),
        velocity_y_(0.0f), pivot_x_(0.5f), pivot_y_(0.5f) {}

  void update(Uint64 delta_time) override {
    // 移動処理
    center_x_ += velocity_x_;
    center_y_ += velocity_y_;

    // 回転処理（delta_timeをミリ秒→秒に変換）
    angle_ += angular_velocity_ * (delta_time / 1000.0f);

    // 角度を0-360度に正規化
    while (angle_ >= 360.0f) angle_ -= 360.0f;
    while (angle_ < 0.0f) angle_ += 360.0f;
  }

  void render(SDL_Renderer* renderer) override {
    // 回転した矩形の4頂点を計算
    SDL_FPoint vertices[4];
    calculateRotatedVertices(vertices);

    // SDL_RenderGeometryで描画（塗りつぶし）
    SDL_Vertex sdl_vertices[4];
    for (int i = 0; i < 4; i++) {
      sdl_vertices[i].position = vertices[i];
      sdl_vertices[i].color.r = color_.r;
      sdl_vertices[i].color.g = color_.g;
      sdl_vertices[i].color.b = color_.b;
      sdl_vertices[i].color.a = color_.a;
      sdl_vertices[i].tex_coord = {0.0f, 0.0f};  // テクスチャなし
    }

    // 2つの三角形で矩形を描画
    int indices[6] = {0, 1, 2, 2, 3, 0};
    SDL_RenderGeometry(
        renderer, nullptr, sdl_vertices, 4, indices, 6);
  }

  /**
   * @brief 回転速度を設定（度/秒）
   * @param angular_vel 角速度（度/秒）
   */
  void setAngularVelocity(float angular_vel) {
    angular_velocity_ = angular_vel;
  }

  /**
   * @brief 回転速度を取得
   * @return 角速度（度/秒）
   */
  float getAngularVelocity() const { return angular_velocity_; }

  /**
   * @brief 移動速度を設定
   * @param vx X方向の速度
   * @param vy Y方向の速度
   */
  void setVelocity(float vx, float vy) {
    velocity_x_ = vx;
    velocity_y_ = vy;
  }

  /**
   * @brief 移動速度を取得
   * @return {vx, vy}
   */
  std::pair<float, float> getVelocity() const {
    return {velocity_x_, velocity_y_};
  }

  /**
   * @brief 位置を設定（中心座標）
   * @param x X座標
   * @param y Y座標
   */
  void setPosition(float x, float y) {
    center_x_ = x;
    center_y_ = y;
  }

  /**
   * @brief 位置を取得（中心座標）
   * @return {x, y}
   */
  std::pair<float, float> getPosition() const { return {center_x_, center_y_}; }

  /**
   * @brief サイズを設定
   * @param w 幅
   * @param h 高さ
   */
  void setSize(float w, float h) {
    width_ = w;
    height_ = h;
  }

  /**
   * @brief サイズを取得
   * @return {w, h}
   */
  std::pair<float, float> getSize() const { return {width_, height_}; }

  /**
   * @brief 回転角度を設定
   * @param angle 角度（度数法）
   */
  void setAngle(float angle) { angle_ = angle; }

  /**
   * @brief 回転角度を取得
   * @return 角度（度数法）
   */
  float getAngle() const { return angle_; }

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
   * @brief 回転の原点（ピボットポイント）を設定
   * @param pivot_x X方向のピボット位置（0.0～1.0、0.0=左端、0.5=中央、1.0=右端）
   * @param pivot_y Y方向のピボット位置（0.0～1.0、0.0=上端、0.5=中央、1.0=下端）
   */
  void setPivot(float pivot_x, float pivot_y) {
    pivot_x_ = pivot_x;
    pivot_y_ = pivot_y;
  }

  /**
   * @brief 回転の原点（ピボットポイント）を取得
   * @return {pivot_x, pivot_y}
   */
  std::pair<float, float> getPivot() const { return {pivot_x_, pivot_y_}; }

 private:
  /**
   * @brief 回転した矩形の4頂点を計算
   * @param vertices 出力先の配列（4要素）
   */
  void calculateRotatedVertices(SDL_FPoint* vertices) {
    // 角度をラジアンに変換
    float rad = angle_ * (M_PI / 180.0f);
    float cos_a = std::cos(rad);
    float sin_a = std::sin(rad);

    // 矩形の半分のサイズ
    float half_w = width_ / 2.0f;
    float half_h = height_ / 2.0f;

    // ピボット位置のオフセット（矩形の中心からの相対位置）
    float pivot_offset_x = (pivot_x_ - 0.5f) * width_;
    float pivot_offset_y = (pivot_y_ - 0.5f) * height_;

    // 回転前の4頂点（中心が原点）
    SDL_FPoint local_vertices[4] = {
        {-half_w, -half_h},  // 左上
        {half_w, -half_h},   // 右上
        {half_w, half_h},    // 右下
        {-half_w, half_h}    // 左下
    };

    // 回転行列を適用して中心座標に移動
    for (int i = 0; i < 4; i++) {
      // 頂点のピボットからの相対位置
      float x = local_vertices[i].x - pivot_offset_x;
      float y = local_vertices[i].y - pivot_offset_y;

      // 回転
      float rotated_x = x * cos_a - y * sin_a;
      float rotated_y = x * sin_a + y * cos_a;

      // ピボット位置に戻して中心座標に移動
      vertices[i].x = rotated_x + pivot_offset_x + center_x_;
      vertices[i].y = rotated_y + pivot_offset_y + center_y_;
    }
  }

  float center_x_, center_y_;      // 中心座標
  float width_, height_;           // サイズ
  SDL_Color color_;                // 色
  float angle_;                    // 回転角度（度数法）
  float angular_velocity_;         // 角速度（度/秒）
  float velocity_x_, velocity_y_;  // 移動速度
  float pivot_x_, pivot_y_;        // 回転の原点（0.0～1.0）
};

}  // namespace MyGame::Entities
