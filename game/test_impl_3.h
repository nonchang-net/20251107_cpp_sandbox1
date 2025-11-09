#pragma once

#include <SDL3/SDL.h>

#include <cmath>
#include <memory>

#include "../game_constant.h"
#include "../game_manager/entity_manager.h"
#include "../game_manager/game_impl.h"
#include "../game_manager/utilities/fps_counter.h"
#include "../game_manager/utilities/texture_loader.h"
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
 * @brief 画面端で跳ね返るコンポーネント
 *
 * 画面端に達したら速度を反転させます。
 * Locator、VelocityMove、RectRendererコンポーネントが必要です。
 */
class BounceOnEdge : public Component {
 public:
  void update(Entity* entity, Uint64 delta_time) override {
    auto* locator = entity->getComponent<Locator>();
    auto* velocity = entity->getComponent<VelocityMove>();
    auto* renderer = entity->getComponent<RectRenderer>();

    if (!locator || !velocity || !renderer) return;

    auto [x, y] = locator->getPosition();
    auto [w, h] = renderer->getSize();
    auto [vx, vy] = velocity->getVelocity();

    bool changed = false;
    if (x < 0 || x + w > 640) {
      vx = -vx;
      changed = true;
    }
    if (y < 0 || y + h > 480) {
      vy = -vy;
      changed = true;
    }

    if (changed) {
      velocity->setVelocity(vx, vy);
    }
  }
};

/**
 * @brief 点滅コンポーネント
 *
 * 周期的に表示フラグを切り替えます。
 */
class Blink : public Component {
 public:
  explicit Blink(Uint64 interval_ms = 500)
      : interval_(interval_ms), timer_(0) {}

  void update(Entity* entity, Uint64 delta_time) override {
    if (entity->getStateFlag(toIndex(TestImpl3StateFlag::Blinking))) {
      timer_ += delta_time;
      if (timer_ > interval_) {
        size_t visible_idx = toIndex(TestImpl3StateFlag::Visible);
        entity->setStateFlag(visible_idx, !entity->getStateFlag(visible_idx));
        timer_ = 0;
      }
    }
  }

 private:
  Uint64 interval_;
  Uint64 timer_;
};

/**
 * @brief ピボットポイントを動的に変更するコンポーネント
 *
 * RotatedRectRendererコンポーネントのピボット位置を周期的に変更します。
 */
class DynamicPivot : public Component {
 public:
  explicit DynamicPivot(Uint64 period_ms = 2000)
      : period_(period_ms), timer_(0) {}

  void update(Entity* entity, Uint64 delta_time) override {
    auto* renderer = entity->getComponent<RotatedRectRenderer>();
    if (!renderer) return;

    // ピボットを周期的に変更（円運動させる）
    timer_ += delta_time;
    float t = timer_ / static_cast<float>(period_);
    float pivot_x = 0.5f + 0.4f * std::cos(t);  // 0.1～0.9の範囲
    float pivot_y = 0.5f + 0.4f * std::sin(t);  // 0.1～0.9の範囲
    renderer->setPivot(pivot_x, pivot_y);
  }

 private:
  Uint64 period_;
  Uint64 timer_;
};

/**
 * @brief EntityManagerのデモ実装
 *
 * レイヤー管理、エンティティの追加・削除、状態フラグのデモを行います。
 */
class TestImpl3 final : public GameImpl {
 private:
  SDL_Renderer* renderer_ = nullptr;
  SDL_Texture* texture_ = nullptr;
  EntityManager entity_manager_;
  Uint64 last_time_;
  Uint64 spawn_timer_;
  Entity* player_ = nullptr;  // プレイヤーエンティティへの参照
  Utilities::FpsCounter fps_counter_;  // FPS計測

  // タイムスケール管理
  float target_timescale_ = 1.0f;  // Tキーで切り替えるタイムスケール（1.0 or 0.5）
  float current_timescale_ = 1.0f;  // 現在のタイムスケール（デバッグ表示用）

 public:
  TestImpl3(SDL_Renderer* renderer)
      : renderer_(renderer), last_time_(SDL_GetTicks()), spawn_timer_(0) {
    // キャンバスサイズを設定（カメラのビューポートと中心位置を調整）
    entity_manager_.setCanvasSize(CANVAS_WIDTH, CANVAS_HEIGHT);

    // 8x8ドット絵表現用のテクスチャ読み込む
    // note: width/heightは今は使わないかも
    auto [texture, width, height] =
        Utilities::load_texture(renderer, "resources/images/nonchang_20240917.png");
    if (!texture) {
      SDL_Log("テクスチャ読み込み失敗");
    } else {
      texture_ = texture;
    }

    // テクスチャ読み込み後にエンティティを初期化
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
          SDL_Log("Cleanup: %zu entities remaining",
                  entity_manager_.getEntityCount());
          break;
        case SDL_SCANCODE_R:
          // Rキーでリセット
          player_ = nullptr;
          entity_manager_.clear();
          initializeEntities();
          break;
        case SDL_SCANCODE_T: {
          // Tキーでタイムスケールを切り替え（1.0 ↔ 0.5）
          target_timescale_ = (target_timescale_ == 1.0f) ? 0.5f : 1.0f;

          // タイムスケール設定要求イベントを発火
          SDL_Event request_event;
          SDL_zero(request_event);
          request_event.type = EVENT_REQUEST_SET_TIMESCALE;
          // floatをポインタで渡すのは危険なので、整数に変換（100倍して整数化）
          request_event.user.code = static_cast<Sint32>(target_timescale_ * 100.0f);
          SDL_PushEvent(&request_event);
          break;
        }
        case SDL_SCANCODE_P: {
          // Pキーでポーズトグル
          SDL_Event request_event;
          SDL_zero(request_event);
          request_event.type = EVENT_REQUEST_TOGGLE_PAUSE;
          SDL_PushEvent(&request_event);
          break;
        }
        default:
          break;
      }
    } else if (event->type == EVENT_TIMESCALE_CHANGED) {
      // タイムスケール変更イベントを受け取る（デバッグ表示用に現在値を保存）
      // GameManagerから現在のタイムスケール値を取得する必要があるが、
      // ここではイベント経由で値を受け取る設計にする
      // event->user.codeに整数化された値が入っている想定
      current_timescale_ = static_cast<float>(event->user.code) / 100.0f;
    }
    return SDL_APP_CONTINUE;
  }

  SDL_AppResult update() override {
    Uint64 current_time = SDL_GetTicks();
    Uint64 delta_time = current_time - last_time_;
    last_time_ = current_time;

    // FPS計測（タイムスケールの影響を受けない）
    fps_counter_.update();

    // タイムスケールを適用したdelta_timeを計算
    Uint64 scaled_delta_time = static_cast<Uint64>(delta_time * current_timescale_);

    // プレイヤー入力処理
    handlePlayerInput();

    // エンティティの更新（タイムスケールを適用）
    entity_manager_.updateAll(scaled_delta_time);

    // 定期的に新しいエンティティを追加（デモ、タイムスケールを適用）
    spawn_timer_ += scaled_delta_time;
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
    SDL_snprintf(buffer, sizeof(buffer), "Entities: %zu",
                 entity_manager_.getEntityCount());
    SDL_RenderDebugText(renderer_, 10, 10, buffer);
    SDL_RenderDebugText(renderer_, 10, 20, "R: Reset, C: Cleanup, Q: Quit");

    SDL_RenderPresent(renderer_);
    return SDL_APP_CONTINUE;
  }

 private:
  /**
   * @brief プレイヤーの入力を処理
   */
  void handlePlayerInput() {
    if (!player_) {
      SDL_Log("player not found.");
      return;
    }
    // VelocityMoveコンポーネントを取得
    auto* velocity = player_->getComponent<VelocityMove>();
    if (!velocity) return;

    // ポーズ中は入力を無効化
    if (current_timescale_ == 0.0f) {
      velocity->setVelocity(0.0f, 0.0f);
      return;
    }

    // キーボード状態を取得
    const bool* keys = SDL_GetKeyboardState(nullptr);

    // 移動速度（ピクセル/秒）
    const float speed = 180.0f;  // 60FPSで3ピクセル/フレーム相当

    // 移動方向を計算
    float vx = 0.0f;
    float vy = 0.0f;

    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
      vy -= speed;
    }
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
      vy += speed;
    }
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
      vx -= speed;
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
      vx += speed;
    }

    // 速度を設定
    velocity->setVelocity(vx, vy);
  }

  void initializeEntities() {
    // レイヤー0: 背景
    auto bg = createRectEntity(0, 0, 0, 640, 480, SDL_Color{30, 30, 60, 255});
    bg->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(bg));

    // レイヤー1: 動く四角形（前景）
    auto rect1 =
        createRectEntity(1, 100, 100, 50, 50, SDL_Color{255, 100, 100, 255});
    rect1->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    if (auto* vel = rect1->getComponent<VelocityMove>()) {
      vel->setVelocity(120.0f, 90.0f);  // 60FPSで2.0, 1.5ピクセル/フレーム相当
    }
    rect1->addComponent(std::make_unique<BounceOnEdge>());
    entity_manager_.addEntity(std::move(rect1));

    auto rect2 =
        createRectEntity(1, 300, 200, 60, 60, SDL_Color{100, 255, 100, 255});
    rect2->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    if (auto* vel = rect2->getComponent<VelocityMove>()) {
      vel->setVelocity(-90.0f, 120.0f);  // 60FPSで-1.5, 2.0ピクセル/フレーム相当
    }
    rect2->addComponent(std::make_unique<BounceOnEdge>());
    entity_manager_.addEntity(std::move(rect2));

    // レイヤー2: 点滅する四角形
    auto blink_rect =
        createRectEntity(2, 250, 150, 80, 80, SDL_Color{100, 100, 255, 255});
    blink_rect->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    blink_rect->setStateFlag(toIndex(TestImpl3StateFlag::Blinking), 1);
    blink_rect->addComponent(std::make_unique<Blink>(500));
    entity_manager_.addEntity(std::move(blink_rect));

    // レイヤー3: 回転する四角形（複数）
    auto rotate_rect1 = createRotateRectEntity(3, 320, 240, 100, 100,
                                               SDL_Color{255, 200, 0, 255});
    rotate_rect1->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    if (auto* ang_vel = rotate_rect1->getComponent<AngularVelocity>()) {
      ang_vel->setAngularVelocity(45.0f);  // 45度/秒で回転
    }
    entity_manager_.addEntity(std::move(rotate_rect1));

    auto rotate_rect2 = createRotateRectEntity(
        3, 500, 100, 60, 80, SDL_Color{0, 255, 200, 255}, 30.0f);
    rotate_rect2->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    if (auto* ang_vel = rotate_rect2->getComponent<AngularVelocity>()) {
      ang_vel->setAngularVelocity(-90.0f);  // -90度/秒で逆回転
    }
    if (auto* vel = rotate_rect2->getComponent<VelocityMove>()) {
      vel->setVelocity(60.0f, 30.0f);  // 移動しながら回転（60FPSで1.0, 0.5ピクセル/フレーム相当）
    }
    entity_manager_.addEntity(std::move(rotate_rect2));

    auto rotate_rect3 = createRotateRectEntity(
        3, 150, 350, 70, 70, SDL_Color{255, 100, 200, 255}, 45.0f);
    rotate_rect3->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    if (auto* ang_vel = rotate_rect3->getComponent<AngularVelocity>()) {
      ang_vel->setAngularVelocity(120.0f);  // 高速回転
    }
    entity_manager_.addEntity(std::move(rotate_rect3));

    // レイヤー4: ピボットポイントのデモ
    // 左上を原点に回転（時計の針のような動き）
    auto pivot_rect1 = createRotateRectEntity(
        4, 100, 100, 120, 10, SDL_Color{255, 255, 100, 255}, 0.0f, 0.0f, 0.0f);
    pivot_rect1->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    if (auto* ang_vel = pivot_rect1->getComponent<AngularVelocity>()) {
      ang_vel->setAngularVelocity(30.0f);
    }
    entity_manager_.addEntity(std::move(pivot_rect1));

    // 底辺中央を原点に回転（振り子のような動き）
    auto pivot_rect2 = createRotateRectEntity(
        4, 400, 100, 15, 100, SDL_Color{100, 255, 255, 255}, 30.0f, 0.5f, 1.0f);
    pivot_rect2->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    if (auto* ang_vel = pivot_rect2->getComponent<AngularVelocity>()) {
      ang_vel->setAngularVelocity(60.0f);
    }
    entity_manager_.addEntity(std::move(pivot_rect2));

    // 右端中央を原点に回転（ドアが開くような動き）
    auto pivot_rect3 = createRotateRectEntity(
        4, 550, 300, 80, 120, SDL_Color{255, 150, 150, 255}, 0.0f, 1.0f, 0.5f);
    pivot_rect3->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    if (auto* ang_vel = pivot_rect3->getComponent<AngularVelocity>()) {
      ang_vel->setAngularVelocity(-25.0f);
    }
    entity_manager_.addEntity(std::move(pivot_rect3));

    // ピボットを動的に変更（複雑な回転運動）
    auto dynamic_pivot = createRotateRectEntity(4, 320, 400, 100, 80,
                                                SDL_Color{200, 150, 255, 255});
    dynamic_pivot->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    if (auto* ang_vel = dynamic_pivot->getComponent<AngularVelocity>()) {
      ang_vel->setAngularVelocity(90.0f);
    }
    dynamic_pivot->addComponent(std::make_unique<DynamicPivot>(2000));
    entity_manager_.addEntity(std::move(dynamic_pivot));

    // レイヤー5: プレイヤーキャラクター
    if (texture_) {
      auto player = std::make_unique<Entity>(5);
      player->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);

      // 座標・スケール
      player->addComponent(std::make_unique<Locator>(320.0f, 240.0f));
      player->addComponent(std::make_unique<Scaler>(4.0f, 4.0f));  // 8x8を32x32に拡大

      // 移動（入力処理はhandlePlayerInput()で行う）
      player->addComponent(std::make_unique<VelocityMove>(0.0f, 0.0f));

      // スプライト描画（初期タイル: x=0, y=1）
      player->addComponent(std::make_unique<SpriteRenderer>(texture_, 8, 0, 1));

      // 歩行アニメーション（x:0,y:1 と x:1,y:1 を500msごとに切り替え）
      std::vector<std::pair<int, int>> walk_frames = {{0, 1}, {1, 1}};
      player->addComponent(std::make_unique<SpriteAnimator>(walk_frames, 500));

      // プレイヤーへの参照を保存してからEntityManagerに追加
      player_ = player.get();
      entity_manager_.addEntity(std::move(player));
    }

    // レイヤー10: UI（最前面）
    // 静的テキスト（ワールド座標、カメラの影響を受ける）
    auto ui_text = createTextEntity(10, 200, 240, "Entity Demo");
    ui_text->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(ui_text));

    // 動的テキスト（FPS表示、UI要素として左上にアンカー）
    static const UIAnchor top_left = UIAnchor::TopLeft;
    auto fps_text = createTextEntity(
        10, 10, 30,
        [this]() {
          static char buffer[64];
          SDL_snprintf(buffer, sizeof(buffer), "FPS: %.1f (%.2fms)",
                       fps_counter_.getFps(),
                       fps_counter_.getAverageFrameTime());
          return std::string(buffer);
        },
        SDL_Color{255, 255, 255, 255}, &top_left);
    fps_text->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(fps_text));

    // タイムスケール表示（UI要素として左上にアンカー）
    auto timescale_text = createTextEntity(
        10, 10, 45,
        [this]() {
          static char buffer[64];
          SDL_snprintf(buffer, sizeof(buffer), "TimeScale: %.2fx",
                       current_timescale_);
          return std::string(buffer);
        },
        SDL_Color{255, 255, 0, 255}, &top_left);
    timescale_text->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);
    entity_manager_.addEntity(std::move(timescale_text));
  }

  void spawnRandomEntity() {
    float x = SDL_randf() * 540.0f + 50.0f;
    float y = SDL_randf() * 380.0f + 50.0f;
    Uint8 r = SDL_rand(256);
    Uint8 g = SDL_rand(256);
    Uint8 b = SDL_rand(256);

    auto entity = createRectEntity(1, x, y, 30, 30, SDL_Color{r, g, b, 255});
    entity->setStateFlag(toIndex(TestImpl3StateFlag::Visible), 1);

    if (auto* vel = entity->getComponent<VelocityMove>()) {
      vel->setVelocity((SDL_randf() - 0.5f) * 240.0f,
                       (SDL_randf() - 0.5f) * 240.0f);  // 60FPSで±2ピクセル/フレーム相当
    }
    entity->addComponent(std::make_unique<BounceOnEdge>());

    entity_manager_.addEntity(std::move(entity));
  }
};

// クラス定義完了後にGameImplementation conceptを満たすことを確認
static_assert(GameImplementation<TestImpl3>,
              "TestImpl3 must satisfy GameImplementation concept");

}  // namespace MyGame
