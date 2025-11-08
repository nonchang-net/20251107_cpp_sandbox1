#pragma once

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <map>
#include <memory>
#include <vector>

namespace MyGame {

/**
 * @brief ゲームエンティティの基底クラス
 *
 * レイヤー番号と状態フラグを持ち、描画処理をサポートします。
 * 状態フラグの意味はゲーム実装側で自由に定義できます。
 */
class Entity {
 public:
  static constexpr size_t MAX_STATE_FLAGS = 8;

  /**
   * @brief コンストラクタ
   * @param layer レイヤー番号（小さいほど背景側）
   */
  explicit Entity(int layer = 0) : layer_(layer), active_(true) {
    state_flags_.fill(0);
  }

  virtual ~Entity() = default;

  /**
   * @brief エンティティの更新処理
   * @param delta_time 前フレームからの経過時間（ミリ秒）
   */
  virtual void update(Uint64 delta_time) = 0;

  /**
   * @brief エンティティの描画処理
   * @param renderer SDLレンダラー
   */
  virtual void render(SDL_Renderer* renderer) = 0;

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

 private:
  int layer_;                                  // レイヤー番号
  bool active_;                                // アクティブフラグ
  std::array<int, MAX_STATE_FLAGS> state_flags_; // 状態フラグ配列
};

/**
 * @brief エンティティを管理するコンテナクラス
 *
 * レイヤー番号順にエンティティを管理し、効率的な追加・削除・列挙をサポート
 */
class EntityManager {
 public:
  using EntityPtr = std::unique_ptr<Entity>;
  using EntityList = std::vector<EntityPtr>;

  EntityManager() = default;
  ~EntityManager() = default;

  /**
   * @brief エンティティを追加
   * @param entity 追加するエンティティ（所有権を移譲）
   */
  void addEntity(EntityPtr entity) {
    int layer = entity->getLayer();
    layers_[layer].push_back(std::move(entity));
  }

  /**
   * @brief アクティブでないエンティティを削除
   *
   * 削除マークされたエンティティをメモリから解放します。
   * パフォーマンス: O(n) - 全エンティティを走査
   */
  void cleanup() {
    for (auto& [layer, entities] : layers_) {
      entities.erase(
          std::remove_if(
              entities.begin(), entities.end(),
              [](const EntityPtr& e) { return !e->isActive(); }),
          entities.end());
    }

    // 空になったレイヤーを削除
    for (auto it = layers_.begin(); it != layers_.end();) {
      if (it->second.empty()) {
        it = layers_.erase(it);
      } else {
        ++it;
      }
    }
  }

  /**
   * @brief すべてのエンティティを更新
   * @param delta_time 前フレームからの経過時間（ミリ秒）
   */
  void updateAll(Uint64 delta_time) {
    for (auto& [layer, entities] : layers_) {
      for (auto& entity : entities) {
        if (entity->isActive()) {
          entity->update(delta_time);
        }
      }
    }
  }

  /**
   * @brief レイヤー順にすべてのエンティティを描画
   *
   * レイヤー番号の小さい順（背景→前景）に描画します。
   * パフォーマンス: O(n) - 全エンティティを走査（ソート済み）
   *
   * @param renderer SDLレンダラー
   * @param visible_flag_index 表示フラグのインデックス（デフォルト: 0）
   */
  void renderAll(SDL_Renderer* renderer, size_t visible_flag_index = 0) {
    // std::mapは自動的にキー（レイヤー番号）順にソートされる
    for (auto& [layer, entities] : layers_) {
      for (auto& entity : entities) {
        if (entity->isActive() && entity->getStateFlag(visible_flag_index)) {
          entity->render(renderer);
        }
      }
    }
  }

  /**
   * @brief すべてのエンティティを削除
   */
  void clear() { layers_.clear(); }

  /**
   * @brief エンティティの総数を取得
   * @return エンティティ数
   */
  size_t getEntityCount() const {
    size_t count = 0;
    for (const auto& [layer, entities] : layers_) {
      count += entities.size();
    }
    return count;
  }

  /**
   * @brief 特定レイヤーのエンティティ数を取得
   * @param layer レイヤー番号
   * @return エンティティ数
   */
  size_t getLayerEntityCount(int layer) const {
    auto it = layers_.find(layer);
    return (it != layers_.end()) ? it->second.size() : 0;
  }

 private:
  // レイヤー番号をキーとしたエンティティリストのマップ
  // std::mapは自動的にソートされるため、レイヤー順の列挙が高速
  std::map<int, EntityList> layers_;
};

} // namespace MyGame
