#pragma once

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "component.h"

namespace MyGame {

// 前方宣言
class Camera2D;

/**
 * @brief ゲームエンティティの基底クラス
 *
 * レイヤー番号と状態フラグを持ち、描画処理をサポートします。
 * 状態フラグの意味はゲーム実装側で自由に定義できます。
 * 親子関係を持ち、階層的な座標系（シーングラフ）を構築できます。
 * コンポーネントベースの設計により、機能を柔軟に組み合わせられます。
 */
class Entity {
 public:
  static constexpr size_t MAX_STATE_FLAGS = 8;

  /**
   * @brief コンストラクタ
   * @param layer レイヤー番号（小さいほど背景側）
   */
  explicit Entity(int layer = 0) : layer_(layer), active_(true), parent_(nullptr) {
    state_flags_.fill(0);
  }

  virtual ~Entity() = default;

  /**
   * @brief エンティティの更新処理
   * @param delta_time 前フレームからの経過時間（ミリ秒）
   *
   * コンポーネントのみで動作する場合は、この関数を実装する必要はありません。
   */
  virtual void update(Uint64 delta_time) {}

  /**
   * @brief エンティティの描画処理
   * @param renderer SDLレンダラー
   *
   * コンポーネントのみで動作する場合は、この関数を実装する必要はありません。
   */
  virtual void render(SDL_Renderer* renderer) {}

  /**
   * @brief レイヤー番号を取得
   * @return レイヤー番号
   */
  int getLayer() const { return layer_; }

  /**
   * @brief レイヤー番号を設定（設定後は再ソートが必要）
   * @param layer レイヤー番号
   */
  void setLayer(int layer) { layer_ = layer; }

  /**
   * @brief エンティティがアクティブかどうか
   * @return アクティブならtrue
   */
  bool isActive() const { return active_; }

  /**
   * @brief エンティティを削除マーク（次のクリーンアップで削除）
   */
  void destroy() { active_ = false; }

  /**
   * @brief 状態フラグを取得
   * @param index フラグのインデックス（0～MAX_STATE_FLAGS-1）
   * @return フラグの値
   */
  int getStateFlag(size_t index) const { return state_flags_[index]; }

  /**
   * @brief 状態フラグを設定
   * @param index フラグのインデックス（0～MAX_STATE_FLAGS-1）
   * @param value 設定する値
   */
  void setStateFlag(size_t index, int value) { state_flags_[index] = value; }

  /**
   * @brief すべての状態フラグを取得
   * @return 状態フラグの配列
   */
  const std::array<int, MAX_STATE_FLAGS>& getStateFlags() const {
    return state_flags_;
  }

  /**
   * @brief コンポーネントを追加
   * @tparam T コンポーネントの型
   * @param component 追加するコンポーネント（所有権を移譲）
   */
  template <typename T>
  void addComponent(std::unique_ptr<T> component) {
    components_[std::type_index(typeid(T))] = std::move(component);
  }

  /**
   * @brief コンポーネントを取得
   * @tparam T コンポーネントの型
   * @return コンポーネントへのポインタ（存在しない場合はnullptr）
   */
  template <typename T>
  T* getComponent() {
    auto it = components_.find(std::type_index(typeid(T)));
    if (it != components_.end()) {
      return static_cast<T*>(it->second.get());
    }
    return nullptr;
  }

  /**
   * @brief コンポーネントを取得（const版）
   * @tparam T コンポーネントの型
   * @return コンポーネントへのポインタ（存在しない場合はnullptr）
   */
  template <typename T>
  const T* getComponent() const {
    auto it = components_.find(std::type_index(typeid(T)));
    if (it != components_.end()) {
      return static_cast<const T*>(it->second.get());
    }
    return nullptr;
  }

  /**
   * @brief コンポーネントを削除
   * @tparam T コンポーネントの型
   */
  template <typename T>
  void removeComponent() {
    components_.erase(std::type_index(typeid(T)));
  }

  /**
   * @brief コンポーネントが存在するか確認
   * @tparam T コンポーネントの型
   * @return 存在する場合true
   */
  template <typename T>
  bool hasComponent() const {
    return components_.find(std::type_index(typeid(T))) != components_.end();
  }

  /**
   * @brief すべてのコンポーネントを取得（内部用）
   * @return コンポーネントマップへの参照
   */
  const std::unordered_map<std::type_index, std::unique_ptr<Component>>&
  getComponents() const {
    return components_;
  }

  /**
   * @brief 子エンティティを追加
   * @param child 追加する子エンティティ（所有権を移譲）
   */
  void addChild(std::unique_ptr<Entity> child) {
    child->parent_ = this;
    children_.push_back(std::move(child));
  }

  /**
   * @brief 子エンティティを削除して返す
   * @param child 削除する子エンティティへのポインタ
   * @return 削除されたエンティティ（所有権を返す）
   */
  std::unique_ptr<Entity> removeChild(Entity* child) {
    auto it = std::find_if(
        children_.begin(), children_.end(),
        [child](const std::unique_ptr<Entity>& e) { return e.get() == child; });

    if (it != children_.end()) {
      std::unique_ptr<Entity> removed = std::move(*it);
      removed->parent_ = nullptr;
      children_.erase(it);
      return removed;
    }
    return nullptr;
  }

  /**
   * @brief 親エンティティを変更
   * @param new_parent 新しい親（nullptrで親から切り離し）
   */
  void setParent(Entity* new_parent) {
    if (parent_) {
      // 現在の親から切り離す
      auto self = parent_->removeChild(this);
      if (new_parent) {
        new_parent->addChild(std::move(self));
      }
    }
  }

  /**
   * @brief 親エンティティを取得
   * @return 親エンティティへのポインタ（親がいない場合はnullptr）
   */
  Entity* getParent() const { return parent_; }

  /**
   * @brief 子エンティティのリストを取得
   * @return 子エンティティのリスト
   */
  const std::vector<std::unique_ptr<Entity>>& getChildren() const {
    return children_;
  }

  /**
   * @brief ローカル座標を設定（Locatorコンポーネントがある場合）
   * @param x X座標（親座標系からの相対位置）
   * @param y Y座標（親座標系からの相対位置）
   */
  void setLocalPosition(float x, float y) {
    if (auto* locator = getComponent<Locator>()) {
      locator->setPosition(x, y);
    }
  }

  /**
   * @brief ローカル座標を取得（Locatorコンポーネントから）
   * @return {x, y} (Locatorがない場合は{0, 0})
   */
  std::pair<float, float> getLocalPosition() const {
    if (const auto* locator = getComponent<Locator>()) {
      return locator->getPosition();
    }
    return {0.0f, 0.0f};
  }

  /**
   * @brief ローカル回転角度を設定（Rotaterコンポーネントがある場合）
   * @param angle 回転角度（度数法、親座標系からの相対角度）
   */
  void setLocalAngle(float angle) {
    if (auto* rotater = getComponent<Rotater>()) {
      rotater->setAngle(angle);
    }
  }

  /**
   * @brief ローカル回転角度を取得（Rotaterコンポーネントから）
   * @return 回転角度（度数法） (Rotaterがない場合は0)
   */
  float getLocalAngle() const {
    if (const auto* rotater = getComponent<Rotater>()) {
      return rotater->getAngle();
    }
    return 0.0f;
  }

  /**
   * @brief ローカルスケールを設定（Scalerコンポーネントがある場合）
   * @param scale_x X方向のスケール
   * @param scale_y Y方向のスケール
   */
  void setLocalScale(float scale_x, float scale_y) {
    if (auto* scaler = getComponent<Scaler>()) {
      scaler->setScale(scale_x, scale_y);
    }
  }

  /**
   * @brief ローカルスケールを取得（Scalerコンポーネントから）
   * @return {scale_x, scale_y} (Scalerがない場合は{1, 1})
   */
  std::pair<float, float> getLocalScale() const {
    if (const auto* scaler = getComponent<Scaler>()) {
      return scaler->getScale();
    }
    return {1.0f, 1.0f};
  }

  /**
   * @brief ワールド座標を取得（親の座標・回転・スケールを考慮）
   * @return {x, y}
   */
  std::pair<float, float> getWorldPosition() const {
    auto [local_x, local_y] = getLocalPosition();

    if (!parent_) {
      return {local_x, local_y};
    }

    auto [parent_x, parent_y] = parent_->getWorldPosition();
    float parent_angle = parent_->getWorldAngle();
    auto [parent_scale_x, parent_scale_y] = parent_->getWorldScale();

    // 親のスケールを適用
    float scaled_x = local_x * parent_scale_x;
    float scaled_y = local_y * parent_scale_y;

    // 親の回転を考慮した座標変換
    float rad = parent_angle * (3.14159265358979323846f / 180.0f);
    float cos_a = std::cos(rad);
    float sin_a = std::sin(rad);

    float rotated_x = scaled_x * cos_a - scaled_y * sin_a;
    float rotated_y = scaled_x * sin_a + scaled_y * cos_a;

    return {parent_x + rotated_x, parent_y + rotated_y};
  }

  /**
   * @brief ワールド回転角度を取得（親の回転を考慮）
   * @return 回転角度（度数法）
   */
  float getWorldAngle() const {
    float local_angle = getLocalAngle();

    if (!parent_) {
      return local_angle;
    }
    return parent_->getWorldAngle() + local_angle;
  }

  /**
   * @brief ワールドスケールを取得（親のスケールを考慮）
   * @return {scale_x, scale_y}
   */
  std::pair<float, float> getWorldScale() const {
    auto [local_scale_x, local_scale_y] = getLocalScale();

    if (!parent_) {
      return {local_scale_x, local_scale_y};
    }

    auto [parent_scale_x, parent_scale_y] = parent_->getWorldScale();
    return {local_scale_x * parent_scale_x, local_scale_y * parent_scale_y};
  }

  /**
   * @brief 子エンティティを含めて更新
   * @param delta_time 前フレームからの経過時間（ミリ秒）
   */
  void updateWithChildren(Uint64 delta_time) {
    if (active_) {
      // 従来のupdate()を呼ぶ（後方互換性）
      update(delta_time);

      // 全コンポーネントのupdate()を呼ぶ
      for (auto& [type, component] : components_) {
        component->update(this, delta_time);
      }

      // 子エンティティも更新
      for (auto& child : children_) {
        child->updateWithChildren(delta_time);
      }
    }
  }

  /**
   * @brief 子エンティティを含めて描画
   * @param renderer SDLレンダラー
   * @param visible_flag_index 表示フラグのインデックス
   */
  void renderWithChildren(
      SDL_Renderer* renderer, size_t visible_flag_index = 0) {
    if (active_ && getStateFlag(visible_flag_index)) {
      // 従来のrender()を呼ぶ（後方互換性）
      render(renderer);

      // 全コンポーネントのrender()を呼ぶ
      for (auto& [type, component] : components_) {
        component->render(this, renderer);
      }
    }

    // 子エンティティも描画
    for (auto& child : children_) {
      child->renderWithChildren(renderer, visible_flag_index);
    }
  }

  /**
   * @brief 描画時のカメラを設定（一時的）
   * @param camera カメラへのポインタ
   */
  void setRenderCamera(const Camera2D* camera) { render_camera_ = camera; }

  /**
   * @brief 描画時のカメラを取得
   * @return カメラへのポインタ
   */
  const Camera2D* getRenderCamera() const { return render_camera_; }

 private:
  int layer_;                                    // レイヤー番号
  bool active_;                                  // アクティブフラグ
  std::array<int, MAX_STATE_FLAGS> state_flags_; // 状態フラグ配列

  // 親子関係
  Entity* parent_;                                 // 親エンティティ（非所有）
  std::vector<std::unique_ptr<Entity>> children_;  // 子エンティティ（所有）

  // コンポーネント管理
  std::unordered_map<std::type_index, std::unique_ptr<Component>> components_;

  // 描画時のカメラ（一時的に設定される、非所有）
  const Camera2D* render_camera_ = nullptr;
};

/**
 * @brief ルートエンティティ（シーングラフの根）
 *
 * 座標は常に(0, 0)に固定され、コンポーネントを一切持ちません。
 * update()とrender()は何もしません（子エンティティのみ処理）。
 * カメラの影響を受けず、ワールド座標系の原点となります。
 */
class RootEntity : public Entity {
 public:
  RootEntity() : Entity(0) {
    // rootはコンポーネントを持たない
    // 座標は常に(0, 0)
    setStateFlag(0, 1);  // デフォルトで表示
  }

  void update(Uint64 delta_time) override {
    // rootは何もしない（子の更新はupdateWithChildren()で行われる）
  }

  void render(SDL_Renderer* renderer) override {
    // rootは何も描画しない（子の描画はrenderWithChildren()で行われる）
  }

  // rootは常に(0, 0)を返す
  std::pair<float, float> getWorldPosition() const { return {0.0f, 0.0f}; }
  float getWorldAngle() const { return 0.0f; }
  std::pair<float, float> getWorldScale() const { return {1.0f, 1.0f}; }
};

/**
 * @brief 2Dカメラ
 *
 * 描画時の視点を管理します。
 * 中心座標、回転、ズーム（スケール）を持ちます。
 * ワールド座標(0, 0)が画面中央になるように変換します。
 */
class Camera2D {
 public:
  Camera2D()
      : center_x_(0.0f), center_y_(0.0f), rotation_(0.0f), zoom_(1.0f),
        viewport_width_(640), viewport_height_(480) {}

  /**
   * @brief カメラの中心座標を設定
   * @param x X座標
   * @param y Y座標
   */
  void setCenter(float x, float y) {
    center_x_ = x;
    center_y_ = y;
  }

  /**
   * @brief カメラの中心座標を取得
   * @return {x, y}
   */
  std::pair<float, float> getCenter() const { return {center_x_, center_y_}; }

  /**
   * @brief カメラの回転角度を設定
   * @param angle 回転角度（度数法）
   */
  void setRotation(float angle) { rotation_ = angle; }

  /**
   * @brief カメラの回転角度を取得
   * @return 回転角度（度数法）
   */
  float getRotation() const { return rotation_; }

  /**
   * @brief カメラのズームを設定
   * @param zoom ズーム倍率（1.0が等倍、2.0で2倍に拡大）
   */
  void setZoom(float zoom) { zoom_ = zoom; }

  /**
   * @brief カメラのズームを取得
   * @return ズーム倍率
   */
  float getZoom() const { return zoom_; }

  /**
   * @brief ビューポートサイズを設定
   * @param width 幅
   * @param height 高さ
   */
  void setViewportSize(float width, float height) {
    viewport_width_ = width;
    viewport_height_ = height;
  }

  /**
   * @brief ワールド座標を画面座標に変換
   * @param world_x ワールドX座標
   * @param world_y ワールドY座標
   * @return {screen_x, screen_y}
   *
   * ワールド座標(0, 0)は画面中央を示します。
   * カメラの位置、回転、ズームを考慮して変換します。
   */
  std::pair<float, float> worldToScreen(float world_x, float world_y) const {
    // カメラからの相対座標
    float rel_x = world_x - center_x_;
    float rel_y = world_y - center_y_;

    // ズームを適用
    float zoomed_x = rel_x * zoom_;
    float zoomed_y = rel_y * zoom_;

    // 回転を適用（TODO: 必要に応じて実装）
    // float rad = rotation_ * (M_PI / 180.0f);
    // ...

    // 画面中央を原点として、画面座標に変換
    float screen_x = viewport_width_ / 2.0f + zoomed_x;
    float screen_y = viewport_height_ / 2.0f + zoomed_y;

    return {screen_x, screen_y};
  }

 private:
  float center_x_, center_y_;  // カメラの中心座標
  float rotation_;             // カメラの回転角度（度数法）
  float zoom_;                 // ズーム倍率
  float viewport_width_, viewport_height_;  // ビューポートサイズ
};

/**
 * @brief エンティティを管理するコンテナクラス
 *
 * ツリー構造（シーングラフ）でエンティティを管理します。
 * 常にrootエンティティを持ち、全てのエンティティはrootの子孫として配置されます。
 * Camera2Dを持ち、描画時の視点変換を行います。
 */
class EntityManager {
 public:
  using EntityPtr = std::unique_ptr<Entity>;
  using EntityList = std::vector<EntityPtr>;

  EntityManager()
      : root_(std::make_unique<RootEntity>()), camera_(std::make_unique<Camera2D>()) {}
  ~EntityManager() = default;

  /**
   * @brief rootエンティティを取得
   * @return rootエンティティへのポインタ
   */
  Entity* getRoot() { return root_.get(); }

  /**
   * @brief カメラを取得
   * @return カメラへのポインタ
   */
  Camera2D* getCamera() { return camera_.get(); }

  /**
   * @brief カメラを取得（const版）
   * @return カメラへのポインタ
   */
  const Camera2D* getCamera() const { return camera_.get(); }

  /**
   * @brief キャンバスサイズを設定
   * @param width キャンバス幅
   * @param height キャンバス高さ
   *
   * カメラのビューポートサイズを設定し、カメラ中心をキャンバス中央に配置します。
   * これにより、ワールド座標(0, 0)が画面左上に、
   * ワールド座標(width/2, height/2)が画面中央にマッピングされます。
   */
  void setCanvasSize(float width, float height) {
    camera_->setViewportSize(width, height);
    camera_->setCenter(width / 2.0f, height / 2.0f);
  }

  /**
   * @brief エンティティをrootの子として追加
   * @param entity 追加するエンティティ（所有権を移譲）
   */
  void addEntity(EntityPtr entity) { root_->addChild(std::move(entity)); }

  /**
   * @brief エンティティを指定した親の子として追加
   * @param parent 親エンティティ
   * @param entity 追加するエンティティ（所有権を移譲）
   */
  void addEntityTo(Entity* parent, EntityPtr entity) {
    parent->addChild(std::move(entity));
  }

  /**
   * @brief アクティブでないエンティティを削除
   *
   * ツリーを再帰的に走査し、削除マークされたエンティティをメモリから解放します。
   */
  void cleanup() { cleanupEntity(root_.get()); }

 private:
  /**
   * @brief エンティティとその子を再帰的にクリーンアップ
   * @param entity クリーンアップ対象のエンティティ
   */
  void cleanupEntity(Entity* entity) {
    auto& children = const_cast<std::vector<std::unique_ptr<Entity>>&>(
        entity->getChildren());

    // 子エンティティを再帰的にクリーンアップ
    for (auto& child : children) {
      cleanupEntity(child.get());
    }

    // アクティブでない子を削除
    children.erase(
        std::remove_if(
            children.begin(), children.end(),
            [](const std::unique_ptr<Entity>& e) { return !e->isActive(); }),
        children.end());
  }

 public:

  /**
   * @brief すべてのエンティティを更新
   * @param delta_time 前フレームからの経過時間（ミリ秒）
   */
  void updateAll(Uint64 delta_time) {
    root_->updateWithChildren(delta_time);
  }

  /**
   * @brief レイヤー順にすべてのエンティティを描画
   *
   * ツリーから全エンティティを集め、レイヤー番号順（背景→前景）に描画します。
   * カメラを使用してワールド座標から画面座標への変換を行います。
   *
   * @param renderer SDLレンダラー
   * @param visible_flag_index 表示フラグのインデックス（デフォルト: 0）
   */
  void renderAll(SDL_Renderer* renderer, size_t visible_flag_index = 0) {
    // ツリーから全エンティティを集める
    std::vector<Entity*> all_entities;
    collectEntities(root_.get(), all_entities);

    // レイヤー順にソート
    std::sort(
        all_entities.begin(), all_entities.end(),
        [](const Entity* a, const Entity* b) {
          return a->getLayer() < b->getLayer();
        });

    // レイヤー順に描画
    for (Entity* entity : all_entities) {
      if (entity->isActive() && entity->getStateFlag(visible_flag_index)) {
        // カメラを設定（コンポーネントが座標変換に使用）
        entity->setRenderCamera(camera_.get());

        // Entity自身の描画（後方互換性）
        entity->render(renderer);

        // コンポーネントの描画
        for (const auto& [type, component] : entity->getComponents()) {
          component->render(entity, renderer);
        }
      }
    }
  }

 private:
  /**
   * @brief エンティティツリーから全エンティティを収集
   * @param entity 収集開始エンティティ
   * @param out_entities 収集先のベクタ
   */
  void collectEntities(Entity* entity, std::vector<Entity*>& out_entities) {
    out_entities.push_back(entity);

    for (const auto& child : entity->getChildren()) {
      collectEntities(child.get(), out_entities);
    }
  }

 public:

  /**
   * @brief すべてのエンティティを削除（rootの子を全てクリア）
   */
  void clear() {
    auto& children = const_cast<std::vector<std::unique_ptr<Entity>>&>(
        root_->getChildren());
    children.clear();
  }

  /**
   * @brief エンティティの総数を取得（root含む）
   * @return エンティティ数
   */
  size_t getEntityCount() const { return countEntities(root_.get()); }

 private:
  /**
   * @brief エンティティツリーの総数を再帰的にカウント
   * @param entity カウント開始エンティティ
   * @return エンティティ数
   */
  size_t countEntities(const Entity* entity) const {
    size_t count = 1; // 自分自身

    for (const auto& child : entity->getChildren()) {
      count += countEntities(child.get());
    }

    return count;
  }

  std::unique_ptr<RootEntity> root_;  // ルートエンティティ
  std::unique_ptr<Camera2D> camera_;  // カメラ
};

// コンポーネントの実装（Entityクラスの完全な定義の後に配置）

// VelocityMoveの実装
inline void VelocityMove::update(Entity* entity, Uint64 delta_time) {
  // Locatorコンポーネントを取得して座標を更新
  if (auto* locator = entity->getComponent<Locator>()) {
    auto [x, y] = locator->getPosition();
    locator->setPosition(x + velocity_x_, y + velocity_y_);
  }
}

// AngularVelocityの実装
inline void AngularVelocity::update(Entity* entity, Uint64 delta_time) {
  // Rotaterコンポーネントを取得して角度を更新
  if (auto* rotater = entity->getComponent<Rotater>()) {
    float angle = rotater->getAngle();
    // delta_timeをミリ秒→秒に変換して角速度を適用
    angle += angular_velocity_ * (delta_time / 1000.0f);

    // 角度を0-360度に正規化
    while (angle >= 360.0f) angle -= 360.0f;
    while (angle < 0.0f) angle += 360.0f;

    rotater->setAngle(angle);
  }
}

// RectRendererの実装
inline void RectRenderer::render(Entity* entity, SDL_Renderer* renderer) {
  // Entityのワールド座標とスケールを取得
  auto [world_x, world_y] = entity->getWorldPosition();
  auto [scale_x, scale_y] = entity->getWorldScale();

  // カメラを使用してワールド座標から画面座標に変換
  float screen_x = world_x;
  float screen_y = world_y;
  if (auto* camera = entity->getRenderCamera()) {
    auto [sx, sy] = camera->worldToScreen(world_x, world_y);
    screen_x = sx;
    screen_y = sy;
  }

  // スケールを適用したサイズ
  float scaled_width = width_ * scale_x;
  float scaled_height = height_ * scale_y;

  // 矩形を描画（左上座標基準）
  SDL_FRect rect{screen_x, screen_y, scaled_width, scaled_height};

  SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, color_.a);
  SDL_RenderFillRect(renderer, &rect);
}

// RotatedRectRendererの実装
inline void RotatedRectRenderer::render(Entity* entity, SDL_Renderer* renderer) {
  // Entityのワールド座標、回転角度、スケールを取得
  auto [world_x, world_y] = entity->getWorldPosition();
  float world_angle = entity->getWorldAngle();
  auto [scale_x, scale_y] = entity->getWorldScale();

  // カメラを使用してワールド座標から画面座標に変換
  float screen_x = world_x;
  float screen_y = world_y;
  if (auto* camera = entity->getRenderCamera()) {
    auto [sx, sy] = camera->worldToScreen(world_x, world_y);
    screen_x = sx;
    screen_y = sy;
  }

  // スケールを適用したサイズ
  float scaled_width = width_ * scale_x;
  float scaled_height = height_ * scale_y;

  // 角度をラジアンに変換
  float rad = world_angle * (3.14159265358979323846f / 180.0f);
  float cos_a = std::cos(rad);
  float sin_a = std::sin(rad);

  // 矩形の半分のサイズ
  float half_w = scaled_width / 2.0f;
  float half_h = scaled_height / 2.0f;

  // ピボット位置のオフセット（矩形の中心からの相対位置）
  float pivot_offset_x = (pivot_x_ - 0.5f) * scaled_width;
  float pivot_offset_y = (pivot_y_ - 0.5f) * scaled_height;

  // 回転前の4頂点（中心が原点）
  SDL_FPoint local_vertices[4] = {
      {-half_w, -half_h},  // 左上
      {half_w, -half_h},   // 右上
      {half_w, half_h},    // 右下
      {-half_w, half_h}    // 左下
  };

  // 回転した頂点を計算
  SDL_FPoint vertices[4];
  for (int i = 0; i < 4; i++) {
    // 頂点のピボットからの相対位置
    float x = local_vertices[i].x - pivot_offset_x;
    float y = local_vertices[i].y - pivot_offset_y;

    // 回転
    float rotated_x = x * cos_a - y * sin_a;
    float rotated_y = x * sin_a + y * cos_a;

    // ピボット位置に戻して画面座標に移動
    vertices[i].x = rotated_x + pivot_offset_x + screen_x;
    vertices[i].y = rotated_y + pivot_offset_y + screen_y;
  }

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
  SDL_RenderGeometry(renderer, nullptr, sdl_vertices, 4, indices, 6);
}

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

// =========================================================================
// SpriteRenderer実装
// =========================================================================

inline void SpriteRenderer::render(Entity* entity, SDL_Renderer* renderer) {
  if (!texture_) return;

  // ワールド座標を取得
  auto [world_x, world_y] = entity->getWorldPosition();
  auto [scale_x, scale_y] = entity->getWorldScale();

  // カメラを使用してワールド座標から画面座標に変換
  float screen_x = world_x;
  float screen_y = world_y;
  if (auto* camera = entity->getRenderCamera()) {
    auto [sx, sy] = camera->worldToScreen(world_x, world_y);
    screen_x = sx;
    screen_y = sy;
  }

  // スプライトシート上のソース矩形を計算
  SDL_FRect src_rect;
  src_rect.x = static_cast<float>(tile_x_ * tile_size_);
  src_rect.y = static_cast<float>(tile_y_ * tile_size_);
  src_rect.w = static_cast<float>(tile_size_);
  src_rect.h = static_cast<float>(tile_size_);

  // 描画先の矩形を計算（スケール適用）
  SDL_FRect dst_rect;
  dst_rect.x = screen_x;
  dst_rect.y = screen_y;
  dst_rect.w = tile_size_ * scale_x;
  dst_rect.h = tile_size_ * scale_y;

  // テクスチャを描画
  SDL_RenderTexture(renderer, texture_, &src_rect, &dst_rect);
}

// =========================================================================
// SpriteAnimator実装
// =========================================================================

inline void SpriteAnimator::update(Entity* entity, Uint64 delta_time) {
  if (frames_.empty()) return;

  // SpriteRendererコンポーネントを取得
  auto* sprite_renderer = entity->getComponent<SpriteRenderer>();
  if (!sprite_renderer) return;

  // タイマーを進める
  timer_ += delta_time;

  // フレーム切り替えが必要かチェック
  if (timer_ >= frame_duration_) {
    timer_ -= frame_duration_;

    // 次のフレームへ
    current_frame_ = (current_frame_ + 1) % frames_.size();

    // SpriteRendererのタイルを更新
    auto [tile_x, tile_y] = frames_[current_frame_];
    sprite_renderer->setTile(tile_x, tile_y);
  }
}

/**
 * @brief RectEntityをコンポーネントで代替するヘルパー関数
 *
 * Locator + VelocityMove + RectRendererを持つEntityを作成します。
 * RectEntityの完全な代替となります。
 *
 * @param layer レイヤー番号
 * @param x X座標
 * @param y Y座標
 * @param w 幅
 * @param h 高さ
 * @param color 色
 * @return 作成されたEntity
 */
inline std::unique_ptr<Entity> createRectEntity(
    int layer, float x, float y, float w, float h, SDL_Color color) {
  auto entity = std::make_unique<Entity>(layer);

  // 座標コンポーネント
  entity->addComponent(std::make_unique<Locator>(x, y));

  // 移動コンポーネント
  entity->addComponent(std::make_unique<VelocityMove>(0.0f, 0.0f));

  // 描画コンポーネント
  entity->addComponent(std::make_unique<RectRenderer>(w, h, color));

  return entity;
}

/**
 * @brief RotateRectEntityをコンポーネントで代替するヘルパー関数
 *
 * Locator + Rotater + VelocityMove + AngularVelocity + RotatedRectRendererを持つEntityを作成します。
 * RotateRectEntityの完全な代替となります。
 *
 * @param layer レイヤー番号
 * @param x X座標
 * @param y Y座標
 * @param w 幅
 * @param h 高さ
 * @param color 色
 * @param angle 初期回転角度（度数法）
 * @param pivot_x X方向のピボット位置（0.0～1.0）
 * @param pivot_y Y方向のピボット位置（0.0～1.0）
 * @return 作成されたEntity
 */
inline std::unique_ptr<Entity> createRotateRectEntity(
    int layer, float x, float y, float w, float h, SDL_Color color,
    float angle = 0.0f, float pivot_x = 0.5f, float pivot_y = 0.5f) {
  auto entity = std::make_unique<Entity>(layer);

  // 座標コンポーネント
  entity->addComponent(std::make_unique<Locator>(x, y));

  // 回転コンポーネント
  entity->addComponent(std::make_unique<Rotater>(angle));

  // 移動コンポーネント
  entity->addComponent(std::make_unique<VelocityMove>(0.0f, 0.0f));

  // 角速度コンポーネント
  entity->addComponent(std::make_unique<AngularVelocity>(0.0f));

  // 回転矩形描画コンポーネント
  entity->addComponent(std::make_unique<RotatedRectRenderer>(w, h, color, pivot_x, pivot_y));

  return entity;
}

/**
 * @brief TextEntityをコンポーネントで代替するヘルパー関数（静的テキスト）
 *
 * Locator + TextRendererを持つEntityを作成します。
 * UIAnchorを指定することで、UI要素として扱うことができます。
 *
 * @param layer レイヤー番号
 * @param x X座標（UIAnchorがある場合はオフセット、ない場合はワールド座標）
 * @param y Y座標（UIAnchorがある場合はオフセット、ない場合はワールド座標）
 * @param text 表示するテキスト
 * @param color 色
 * @param anchor UIアンカー（nullptrの場合はゲーム要素として扱う）
 * @return 作成されたEntity
 */
inline std::unique_ptr<Entity> createTextEntity(
    int layer, float x, float y, const std::string& text,
    SDL_Color color = {255, 255, 255, 255},
    const UIAnchor* anchor = nullptr) {
  auto entity = std::make_unique<Entity>(layer);

  // 座標コンポーネント
  entity->addComponent(std::make_unique<Locator>(x, y));

  // テキスト描画コンポーネント
  entity->addComponent(std::make_unique<TextRenderer>(text, color));

  // UIアンカーが指定されている場合は追加
  if (anchor) {
    entity->addComponent(std::make_unique<UIAnchorComponent>(*anchor));
  }

  return entity;
}

/**
 * @brief TextEntityをコンポーネントで代替するヘルパー関数（動的テキスト）
 *
 * Locator + TextRendererを持つEntityを作成します。
 * テキストを返す関数を渡すことで、動的に更新されるテキストを表示できます。
 *
 * @param layer レイヤー番号
 * @param x X座標（UIAnchorがある場合はオフセット、ない場合はワールド座標）
 * @param y Y座標（UIAnchorがある場合はオフセット、ない場合はワールド座標）
 * @param text_provider テキストを返す関数
 * @param color 色
 * @param anchor UIアンカー（nullptrの場合はゲーム要素として扱う）
 * @return 作成されたEntity
 */
inline std::unique_ptr<Entity> createTextEntity(
    int layer, float x, float y, std::function<std::string()> text_provider,
    SDL_Color color = {255, 255, 255, 255},
    const UIAnchor* anchor = nullptr) {
  auto entity = std::make_unique<Entity>(layer);

  // 座標コンポーネント
  entity->addComponent(std::make_unique<Locator>(x, y));

  // テキスト描画コンポーネント（動的）
  entity->addComponent(std::make_unique<TextRenderer>(text_provider, color));

  // UIアンカーが指定されている場合は追加
  if (anchor) {
    entity->addComponent(std::make_unique<UIAnchorComponent>(*anchor));
  }

  return entity;
}

}  // namespace MyGame
