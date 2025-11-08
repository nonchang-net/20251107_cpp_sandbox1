#pragma once

#include <SDL3/SDL.h>

#include <cmath>
#include <memory>

#include "../game_manager/entities/rect_entity.h"
#include "../game_manager/entities/rotate_rect_entity.h"
#include "../game_manager/entities/text_entity.h"
#include "../game_manager/entity_manager.h"
#include "../game_manager/game_impl.h"

namespace MyGame {

/**
 * @brief TestImpl3で使用するエンティティ状態フラグ
 *
 * ゲーム実装固有の状態フラグを定義します。
 */
enum class TestImpl3StateFlag : size_t {
  Visible = 0,   // 表示中
  Hidden = 1,    // 一時非表示
  Blinking = 2,  // 点滅状態
};

// 状態フラグを使いやすくするヘルパー関数
inline size_t toIndex(TestImpl3StateFlag flag) {
  return static_cast<size_t>(flag);
}

/**
 * @brief 点滅機能付き矩形エンティティ（デモ用）
 *
 * 汎用的なRectEntityを継承して点滅機能を追加
 */
class BlinkingRectEntity : public Entities::RectEntity {
 public:
  BlinkingRectEntity(
      int layer, float x, float y, float w, float h, SDL_Color color)
      : RectEntity(layer, x, y, w, h, color), blink_timer_(0) {
    // デフォルトで表示状態
    setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
  }

  void update(Uint64 delta_time) override {
    // 基底クラスの更新（移動処理）
    RectEntity::update(delta_time);

    // 画面端で跳ね返り
    auto [x, y] = getPosition();
    auto [w, h] = getSize();
    auto [vx, vy] = getVelocity();

    if (x < 0 || x + w > 640) {
      setVelocity(-vx, vy);
    }
    if (y < 0 || y + h > 480) {
      setVelocity(vx, -vy);
    }

    // 点滅処理
    if (getStateFlag(toIndex(TestImpl3StateFlag::Blinking))) {
      blink_timer_ += delta_time;
      if (blink_timer_ > 500) {  // 500ms周期で点滅
        setStateFlag(
            toIndex(TestImpl3StateFlag::Visible),
            !getStateFlag(toIndex(TestImpl3StateFlag::Visible)));
        blink_timer_ = 0;
      }
    }
  }

 private:
  Uint64 blink_timer_;
};

/**
 * @brief ピボットポイントを動的に変更する矩形エンティティ（デモ用）
 *
 * 汎用的なRotateRectEntityを継承してピボット変更機能を追加
 */
class DynamicPivotRectEntity : public Entities::RotateRectEntity {
 public:
  DynamicPivotRectEntity(
      int layer, float x, float y, float w, float h, SDL_Color color)
      : RotateRectEntity(layer, x, y, w, h, color), pivot_timer_(0) {}

  void update(Uint64 delta_time) override {
    // 基底クラスの更新（回転・移動処理）
    RotateRectEntity::update(delta_time);

    // ピボットを周期的に変更（円運動させる）
    pivot_timer_ += delta_time;
    float t = pivot_timer_ / 2000.0f;  // 2秒周期
    float pivot_x = 0.5f + 0.4f * std::cos(t);  // 0.1～0.9の範囲
    float pivot_y = 0.5f + 0.4f * std::sin(t);  // 0.1～0.9の範囲
    setPivot(pivot_x, pivot_y);
  }

 private:
  Uint64 pivot_timer_;
};

/**
 * @brief EntityManagerのデモ実装
 *
 * レイヤー管理、エンティティの追加・削除、状態フラグのデモを行います。
 */
class TestImpl3 final : public GameImpl {
 private:
  SDL_Renderer* renderer_ = nullptr;
  EntityManager entity_manager_;
  Uint64 last_time_;
  Uint64 spawn_timer_;

 public:
  TestImpl3(SDL_Renderer* renderer)
      : renderer_(renderer), last_time_(SDL_GetTicks()), spawn_timer_(0) {
    initializeEntities();
  }

  SDL_AppResult handleSdlEvent(SDL_Event* event) override {
    if (event->type == SDL_EVENT_KEY_DOWN) {
      switch (event->key.scancode) {
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_Q:
          return SDL_APP_SUCCESS;
        case SDL_SCANCODE_C:
          // Cキーでクリーンアップ（削除マークされたエンティティを削除）
          entity_manager_.cleanup();
          SDL_Log(
              "Cleanup: %zu entities remaining",
              entity_manager_.getEntityCount());
          break;
        case SDL_SCANCODE_R:
          // Rキーでリセット
          entity_manager_.clear();
          initializeEntities();
          break;
        default:
          break;
      }
    }
    return SDL_APP_CONTINUE;
  }

  SDL_AppResult update() override {
    Uint64 current_time = SDL_GetTicks();
    Uint64 delta_time = current_time - last_time_;
    last_time_ = current_time;

    // エンティティの更新
    entity_manager_.updateAll(delta_time);

    // 定期的に新しいエンティティを追加（デモ）
    spawn_timer_ += delta_time;
    if (spawn_timer_ > 2000 && entity_manager_.getEntityCount() < 50) {
      spawnRandomEntity();
      spawn_timer_ = 0;
    }

    // 描画
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    // レイヤー順に描画（Visibleフラグをチェック）
    entity_manager_.renderAll(renderer_, toIndex(TestImpl3StateFlag::Visible));

    // デバッグ情報
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    char buffer[64];
    SDL_snprintf(
        buffer, sizeof(buffer), "Entities: %zu",
        entity_manager_.getEntityCount());
    SDL_RenderDebugText(renderer_, 10, 10, buffer);
    SDL_RenderDebugText(renderer_, 10, 20, "R: Reset, C: Cleanup, Q: Quit");

    SDL_RenderPresent(renderer_);
    return SDL_APP_CONTINUE;
  }

 private:
  void initializeEntities() {
    // レイヤー0: 背景
    auto bg = std::make_unique<Entities::RectEntity>(
        0, 0, 0, 640, 480, SDL_Color{30, 30, 60, 255});
    bg->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(bg));

    // レイヤー1: 動く四角形（前景）
    auto rect1 = std::make_unique<BlinkingRectEntity>(
        1, 100, 100, 50, 50, SDL_Color{255, 100, 100, 255});
    rect1->setVelocity(2.0f, 1.5f);
    entity_manager_.addEntity(std::move(rect1));

    auto rect2 = std::make_unique<BlinkingRectEntity>(
        1, 300, 200, 60, 60, SDL_Color{100, 255, 100, 255});
    rect2->setVelocity(-1.5f, 2.0f);
    entity_manager_.addEntity(std::move(rect2));

    // レイヤー2: 点滅する四角形
    auto blink_rect = std::make_unique<BlinkingRectEntity>(
        2, 250, 150, 80, 80, SDL_Color{100, 100, 255, 255});
    blink_rect->setStateFlag(toIndex(TestImpl3StateFlag::Blinking), 1);
    entity_manager_.addEntity(std::move(blink_rect));

    // レイヤー3: 回転する四角形（複数）
    auto rotate_rect1 = std::make_unique<Entities::RotateRectEntity>(
        3, 320, 240, 100, 100, SDL_Color{255, 200, 0, 255});
    rotate_rect1->setAngularVelocity(45.0f);  // 45度/秒で回転
    rotate_rect1->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(rotate_rect1));

    auto rotate_rect2 = std::make_unique<Entities::RotateRectEntity>(
        3, 500, 100, 60, 80, SDL_Color{0, 255, 200, 255}, 30.0f);
    rotate_rect2->setAngularVelocity(-90.0f);  // -90度/秒で逆回転
    rotate_rect2->setVelocity(1.0f, 0.5f);  // 移動しながら回転
    rotate_rect2->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(rotate_rect2));

    auto rotate_rect3 = std::make_unique<Entities::RotateRectEntity>(
        3, 150, 350, 70, 70, SDL_Color{255, 100, 200, 255}, 45.0f);
    rotate_rect3->setAngularVelocity(120.0f);  // 高速回転
    rotate_rect3->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(rotate_rect3));

    // レイヤー4: ピボットポイントのデモ
    // 左上を原点に回転（時計の針のような動き）
    auto pivot_rect1 = std::make_unique<Entities::RotateRectEntity>(
        4, 100, 100, 120, 10, SDL_Color{255, 255, 100, 255});
    pivot_rect1->setPivot(0.0f, 0.0f);  // 左上を原点に
    pivot_rect1->setAngularVelocity(30.0f);
    pivot_rect1->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(pivot_rect1));

    // 底辺中央を原点に回転（振り子のような動き）
    auto pivot_rect2 = std::make_unique<Entities::RotateRectEntity>(
        4, 400, 100, 15, 100, SDL_Color{100, 255, 255, 255}, 30.0f);
    pivot_rect2->setPivot(0.5f, 1.0f);  // 底辺中央を原点に
    pivot_rect2->setAngularVelocity(60.0f);
    pivot_rect2->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(pivot_rect2));

    // 右端中央を原点に回転（ドアが開くような動き）
    auto pivot_rect3 = std::make_unique<Entities::RotateRectEntity>(
        4, 550, 300, 80, 120, SDL_Color{255, 150, 150, 255});
    pivot_rect3->setPivot(1.0f, 0.5f);  // 右端中央を原点に
    pivot_rect3->setAngularVelocity(-25.0f);
    pivot_rect3->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(pivot_rect3));

    // ピボットを動的に変更（複雑な回転運動）
    auto dynamic_pivot = std::make_unique<DynamicPivotRectEntity>(
        4, 320, 400, 100, 80, SDL_Color{200, 150, 255, 255});
    dynamic_pivot->setAngularVelocity(90.0f);
    dynamic_pivot->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(dynamic_pivot));

    // レイヤー10: UI（最前面）
    // 静的テキスト
    auto ui_text = std::make_unique<Entities::TextEntity>(
        10, 200, 240, "Entity Demo");
    ui_text->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(ui_text));

    // 動的テキスト（FPS表示のデモ）
    auto fps_text = std::make_unique<Entities::TextEntity>(
        10, 10, 30,
        [this]() {
          static char buffer[64];
          SDL_snprintf(
              buffer, sizeof(buffer), "FPS: ~%.0f",
              1000.0f / SDL_max(1, SDL_GetTicks() - last_time_));
          return std::string(buffer);
        });
    fps_text->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(fps_text));
  }

  void spawnRandomEntity() {
    float x = SDL_randf() * 540.0f + 50.0f;
    float y = SDL_randf() * 380.0f + 50.0f;
    Uint8 r = SDL_rand(256);
    Uint8 g = SDL_rand(256);
    Uint8 b = SDL_rand(256);

    auto entity = std::make_unique<BlinkingRectEntity>(
        1, x, y, 30, 30, SDL_Color{r, g, b, 255});
    entity->setVelocity(
        (SDL_randf() - 0.5f) * 4.0f, (SDL_randf() - 0.5f) * 4.0f);
    entity_manager_.addEntity(std::move(entity));
  }
};

// クラス定義完了後にGameImplementation conceptを満たすことを確認
static_assert(
    GameImplementation<TestImpl3>,
    "TestImpl3 must satisfy GameImplementation concept");

}  // namespace MyGame
