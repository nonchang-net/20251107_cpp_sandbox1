#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace MyGame {

// 前方宣言
class Entity;

/**
 * @brief コンポーネントの基底クラス
 *
 * Entityに付与される機能の単位です。
 * 座標、回転、描画などの機能を個別のコンポーネントとして実装します。
 */
class Component {
 public:
  virtual ~Component() = default;

  /**
   * @brief コンポーネントの更新処理
   * @param entity このコンポーネントが所属するEntity
   * @param delta_time 前フレームからの経過時間（ミリ秒）
   */
  virtual void update(Entity* entity, Uint64 delta_time) {}

  /**
   * @brief コンポーネントの描画処理
   * @param entity このコンポーネントが所属するEntity
   * @param renderer SDLレンダラー
   */
  virtual void render(Entity* entity, SDL_Renderer* renderer) {}
};

/**
 * @brief 2D座標を保持するコンポーネント
 *
 * Entityのローカル座標（親からの相対位置）を管理します。
 * このコンポーネントがない場合、座標は(0, 0)として扱われます。
 */
class Locator : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param x X座標（親からの相対位置）
   * @param y Y座標（親からの相対位置）
   */
  Locator(float x = 0.0f, float y = 0.0f) : x_(x), y_(y) {}

  /**
   * @brief 座標を設定
   * @param x X座標
   * @param y Y座標
   */
  void setPosition(float x, float y) {
    x_ = x;
    y_ = y;
  }

  /**
   * @brief 座標を取得
   * @return {x, y}
   */
  std::pair<float, float> getPosition() const { return {x_, y_}; }

  /**
   * @brief X座標を取得
   * @return X座標
   */
  float getX() const { return x_; }

  /**
   * @brief Y座標を取得
   * @return Y座標
   */
  float getY() const { return y_; }

 private:
  float x_, y_;
};

/**
 * @brief 2D回転を保持するコンポーネント
 *
 * Entityのローカル回転角度（親からの相対角度）を管理します。
 * このコンポーネントがない場合、回転角度は0として扱われます。
 */
class Rotater : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param angle 回転角度（度数法）
   */
  explicit Rotater(float angle = 0.0f) : angle_(angle) {}

  /**
   * @brief 回転角度を設定
   * @param angle 回転角度（度数法）
   */
  void setAngle(float angle) { angle_ = angle; }

  /**
   * @brief 回転角度を取得
   * @return 回転角度（度数法）
   */
  float getAngle() const { return angle_; }

 private:
  float angle_;
};

/**
 * @brief 2Dスケールを保持するコンポーネント
 *
 * EntityのX方向・Y方向のスケールを管理します。
 * このコンポーネントがない場合、スケールは(1.0, 1.0)として扱われます。
 */
class Scaler : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param scale_x X方向のスケール
   * @param scale_y Y方向のスケール
   */
  Scaler(float scale_x = 1.0f, float scale_y = 1.0f)
      : scale_x_(scale_x), scale_y_(scale_y) {}

  /**
   * @brief スケールを設定
   * @param scale_x X方向のスケール
   * @param scale_y Y方向のスケール
   */
  void setScale(float scale_x, float scale_y) {
    scale_x_ = scale_x;
    scale_y_ = scale_y;
  }

  /**
   * @brief スケールを取得
   * @return {scale_x, scale_y}
   */
  std::pair<float, float> getScale() const { return {scale_x_, scale_y_}; }

  /**
   * @brief X方向のスケールを取得
   * @return X方向のスケール
   */
  float getScaleX() const { return scale_x_; }

  /**
   * @brief Y方向のスケールを取得
   * @return Y方向のスケール
   */
  float getScaleY() const { return scale_y_; }

 private:
  float scale_x_, scale_y_;
};

/**
 * @brief 速度による移動を行うコンポーネント
 *
 * 毎フレーム、Locatorコンポーネントの座標を速度分だけ移動させます。
 * Locatorコンポーネントが必要です。
 */
class VelocityMove : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param vx X方向の速度
   * @param vy Y方向の速度
   */
  VelocityMove(float vx = 0.0f, float vy = 0.0f)
      : velocity_x_(vx), velocity_y_(vy) {}

  void update(Entity* entity, Uint64 delta_time) override;

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

 private:
  float velocity_x_, velocity_y_;
};

/**
 * @brief 角速度による回転を行うコンポーネント
 *
 * 毎フレーム、Rotaterコンポーネントの角度を角速度分だけ回転させます。
 * Rotaterコンポーネントが必要です。
 */
class AngularVelocity : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param angular_vel 角速度（度/秒）
   */
  explicit AngularVelocity(float angular_vel = 0.0f)
      : angular_velocity_(angular_vel) {}

  void update(Entity* entity, Uint64 delta_time) override;

  /**
   * @brief 角速度を設定
   * @param angular_vel 角速度（度/秒）
   */
  void setAngularVelocity(float angular_vel) { angular_velocity_ = angular_vel; }

  /**
   * @brief 角速度を取得
   * @return 角速度（度/秒）
   */
  float getAngularVelocity() const { return angular_velocity_; }

 private:
  float angular_velocity_;
};

/**
 * @brief 矩形を描画するコンポーネント
 *
 * 指定されたサイズと色で矩形を描画します。
 * Entityのワールド座標・回転・スケールを考慮して描画されます。
 */
class RectRenderer : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param width 幅
   * @param height 高さ
   * @param color 色
   */
  RectRenderer(float width, float height, SDL_Color color)
      : width_(width), height_(height), color_(color) {}

  void render(Entity* entity, SDL_Renderer* renderer) override;

  /**
   * @brief サイズを設定
   * @param width 幅
   * @param height 高さ
   */
  void setSize(float width, float height) {
    width_ = width;
    height_ = height;
  }

  /**
   * @brief サイズを取得
   * @return {width, height}
   */
  std::pair<float, float> getSize() const { return {width_, height_}; }

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

 private:
  float width_, height_;
  SDL_Color color_;
};

/**
 * @brief 回転する矩形を描画するコンポーネント
 *
 * ピボットポイントを考慮して回転した矩形を描画します。
 * Entityのワールド座標・回転・スケールを考慮して描画されます。
 */
class RotatedRectRenderer : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param width 幅
   * @param height 高さ
   * @param color 色
   * @param pivot_x X方向のピボット位置（0.0～1.0、0.0=左端、0.5=中央、1.0=右端）
   * @param pivot_y Y方向のピボット位置（0.0～1.0、0.0=上端、0.5=中央、1.0=下端）
   */
  RotatedRectRenderer(float width, float height, SDL_Color color,
                      float pivot_x = 0.5f, float pivot_y = 0.5f)
      : width_(width), height_(height), color_(color),
        pivot_x_(pivot_x), pivot_y_(pivot_y) {}

  void render(Entity* entity, SDL_Renderer* renderer) override;

  /**
   * @brief サイズを設定
   * @param width 幅
   * @param height 高さ
   */
  void setSize(float width, float height) {
    width_ = width;
    height_ = height;
  }

  /**
   * @brief サイズを取得
   * @return {width, height}
   */
  std::pair<float, float> getSize() const { return {width_, height_}; }

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
   * @param pivot_x X方向のピボット位置（0.0～1.0）
   * @param pivot_y Y方向のピボット位置（0.0～1.0）
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
  float width_, height_;
  SDL_Color color_;
  float pivot_x_, pivot_y_;  // 回転の原点（0.0～1.0）
};

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

/**
 * @brief スプライトシートから特定のタイルを描画するコンポーネント
 *
 * テクスチャをグリッド状のタイルに分割し、指定した座標のタイルを描画します。
 * Entityのワールド座標・スケールを考慮して描画されます。
 * 左右反転機能をサポートします。
 */
class SpriteRenderer : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param texture スプライトシートのテクスチャ
   * @param tile_size タイル1つのサイズ（ピクセル）
   * @param tile_x 描画するタイルのX座標（グリッド座標）
   * @param tile_y 描画するタイルのY座標（グリッド座標）
   */
  SpriteRenderer(SDL_Texture* texture, int tile_size, int tile_x = 0,
                 int tile_y = 0, bool flip_horizontal = false)
      : texture_(texture), tile_size_(tile_size), tile_x_(tile_x),
        tile_y_(tile_y), flip_horizontal_(flip_horizontal) {}

  void render(Entity* entity, SDL_Renderer* renderer) override;

  /**
   * @brief 描画するタイルを設定
   * @param tile_x タイルのX座標（グリッド座標）
   * @param tile_y タイルのY座標（グリッド座標）
   */
  void setTile(int tile_x, int tile_y) {
    tile_x_ = tile_x;
    tile_y_ = tile_y;
  }

  /**
   * @brief 描画するタイルを取得
   * @return {tile_x, tile_y}
   */
  std::pair<int, int> getTile() const { return {tile_x_, tile_y_}; }

  /**
   * @brief テクスチャを設定
   * @param texture 新しいテクスチャ
   */
  void setTexture(SDL_Texture* texture) { texture_ = texture; }

  /**
   * @brief テクスチャを取得
   * @return テクスチャ
   */
  SDL_Texture* getTexture() const { return texture_; }

  /**
   * @brief 左右反転を設定
   * @param flip 反転するかどうか
   */
  void setFlipHorizontal(bool flip) { flip_horizontal_ = flip; }

  /**
   * @brief 左右反転を取得
   * @return 反転しているかどうか
   */
  bool isFlipHorizontal() const { return flip_horizontal_; }

 private:
  SDL_Texture* texture_;     // スプライトシートのテクスチャ
  int tile_size_;            // タイル1つのサイズ
  int tile_x_, tile_y_;      // 描画するタイルのグリッド座標
  bool flip_horizontal_;     // 左右反転フラグ
};

/**
 * @brief 向きごとのスプライトフレームセットを管理するコンポーネント
 *
 * DirectionComponentの向きに応じて、SpriteAnimatorとSpriteRendererを自動更新します。
 * DirectionComponent、SpriteAnimator、SpriteRendererコンポーネントが必要です。
 */
class DirectionalSpriteAnimator : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param down_frames 下向きのフレームリスト
   * @param up_frames 上向きのフレームリスト
   * @param right_frames 右向きのフレームリスト
   * @param left_frames 左向きのフレームリスト（空の場合は右向きを左右反転）
   */
  DirectionalSpriteAnimator(
      const std::vector<std::pair<int, int>>& down_frames,
      const std::vector<std::pair<int, int>>& up_frames,
      const std::vector<std::pair<int, int>>& right_frames,
      const std::vector<std::pair<int, int>>& left_frames = {})
      : down_frames_(down_frames), up_frames_(up_frames),
        right_frames_(right_frames), left_frames_(left_frames),
        current_direction_(Direction::Down) {}

  void update(Entity* entity, Uint64 delta_time) override;

  /**
   * @brief 各向きのフレームリストを設定
   */
  void setDownFrames(const std::vector<std::pair<int, int>>& frames) {
    down_frames_ = frames;
  }
  void setUpFrames(const std::vector<std::pair<int, int>>& frames) {
    up_frames_ = frames;
  }
  void setRightFrames(const std::vector<std::pair<int, int>>& frames) {
    right_frames_ = frames;
  }
  void setLeftFrames(const std::vector<std::pair<int, int>>& frames) {
    left_frames_ = frames;
  }

 private:
  std::vector<std::pair<int, int>> down_frames_;   // 下向きフレーム
  std::vector<std::pair<int, int>> up_frames_;     // 上向きフレーム
  std::vector<std::pair<int, int>> right_frames_;  // 右向きフレーム
  std::vector<std::pair<int, int>> left_frames_;   // 左向きフレーム（空なら右向きを反転）
  Direction current_direction_;  // 前回の向き（変化検出用）
};

/**
 * @brief スプライトアニメーションを管理するコンポーネント
 *
 * フレームリストを順に切り替えてアニメーションします。
 * SpriteRendererコンポーネントが必要です。
 */
class SpriteAnimator : public Component {
 public:
  /**
   * @brief コンストラクタ
   * @param frames アニメーションフレームのリスト（タイルのグリッド座標）
   * @param frame_duration 1フレームの表示時間（ミリ秒）
   */
  SpriteAnimator(const std::vector<std::pair<int, int>>& frames,
                 Uint64 frame_duration = 500)
      : frames_(frames), frame_duration_(frame_duration), current_frame_(0),
        timer_(0) {}

  void update(Entity* entity, Uint64 delta_time) override;

  /**
   * @brief フレームリストを設定
   * @param frames 新しいフレームリスト
   */
  void setFrames(const std::vector<std::pair<int, int>>& frames) {
    frames_ = frames;
    current_frame_ = 0;
    timer_ = 0;
  }

  /**
   * @brief フレーム表示時間を設定
   * @param duration 1フレームの表示時間（ミリ秒）
   */
  void setFrameDuration(Uint64 duration) { frame_duration_ = duration; }

  /**
   * @brief 現在のフレーム番号を取得
   * @return フレーム番号
   */
  size_t getCurrentFrame() const { return current_frame_; }

 private:
  std::vector<std::pair<int, int>> frames_;  // フレームリスト（タイル座標）
  Uint64 frame_duration_;                    // 1フレームの表示時間
  size_t current_frame_;                     // 現在のフレーム番号
  Uint64 timer_;                             // タイマー
};

}  // namespace MyGame
