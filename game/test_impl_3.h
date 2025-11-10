#pragma once

#include <SDL3/SDL.h>

#include <cmath>
#include <memory>

#include "../game_constant.h"
#include "../game_manager/entity_manager.h"
#include "../game_manager/game_impl.h"
#include "../game_manager/utilities/fps_counter.h"
#include "../game_manager/utilities/texture_loader.h"
#include "../game_manager/sound_synthesizer.h"

namespace MyGame {

// MMLパーサーはconstexpr対応（コンパイル時評価）
// FixedNoteSequence（固定長配列）を使用することで、完全なconstexpr変数として保存可能

// 初期テスト用MML曲データ
// namespace MMLPresets {
//   constexpr auto frog_song = "t120 o4 l4 @0 cdefedec"_mml;
//   constexpr auto oscillator_demo = "t140 o4 l8 @1 cdefgab>c r4 @2 <bagfedc"_mml;
//   constexpr auto volume_demo = "t120 o4 l8 @0 v15 cccc v12 cccc v8 cccc v4 cccc v0 cccc"_mml;

//   // マルチトラック用MMLプリセット
//   // BGM1: 2トラック構成（メロディー + ベース）
//   constexpr auto bgm1_melody = "t140 o5 l8 @0 v12 cdefg4 agfe d4.r4 efga b4>cd<b a4.r4"_mml;
//   constexpr auto bgm1_bass = "t140 o3 l4 @1 v10 c2c2 g2g2 f2a2 d2d2"_mml;

//   // BGM2: 3トラック構成（メロディー + 和音1 + 和音2）
//   constexpr auto bgm2_melody = "t120 o5 l8 @0 v15 gagf e4d4 c4<b4 a2>c2"_mml;
//   constexpr auto bgm2_chord1 = "t120 o4 l4 @1 v10 e2c2 g2e2"_mml;
//   constexpr auto bgm2_chord2 = "t120 o3 l4 @2 v10 c2a2 e2c2"_mml;

//   // BGM3: 2トラック構成（リズミカルな曲）
//   constexpr auto bgm3_lead = "t160 o4 l16 @1 v14 ccccdddd eeeegggg aaaab4 >c4<b4a4"_mml;
//   constexpr auto bgm3_backing = "t160 o3 l8 @2 v11 c4c4d4d4 e4e4g4g4 a4a4b4b4 >c4<b4a4g4"_mml;

//   // コンパイル時評価の確認（static_assertでコンパイル時評価されてることを確認）
//   static_assert(frog_song.size() == 8, "Frog song should have 8 notes");
//   static_assert(oscillator_demo.size() == 16, "Oscillator demo should have 16 notes (including rest)");
//   static_assert(volume_demo.size() == 20, "Volume demo should have 20 notes");
// }

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

  // サウンドシンセサイザー
  std::unique_ptr<SimpleSynthesizer> synthesizer_;
  std::unique_ptr<Sequencer> sequencer_;
  WaveType ocillatorWaveType_ = WaveType::Sine;

  // std::unique_ptr<SimpleSynthesizer> synthesizer1_;
  // std::unique_ptr<Sequencer> sequencer1_;
  // std::unique_ptr<SimpleSynthesizer> synthesizer2_;
  // std::unique_ptr<Sequencer> sequencer2_;

  // float sequencer_vol_ = 0.8f;

  // BGMマネージャー
  BGMManager bgm_manager_;
  float bgm_master_volume_ = 0.6f;


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

    // サウンドエフェクト用シンセサイザーを初期化
    synthesizer_ = std::make_unique<SimpleSynthesizer>(44100);
    sequencer_ = std::make_unique<Sequencer>(synthesizer_.get(), 120.0f);

    // BGMマネージャーの初期化
    initializeBGMManager();

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

        // サウンドエフェクトテスト（検討中）
        case SDL_SCANCODE_SPACE: {
          // Spaceキー: A4（440Hz）のテストトーン
          synthesizer_->getOscillator().setWaveType(ocillatorWaveType_);
          synthesizer_->noteOn(440.0f, 0.5f);
          break;
        }
        // case SDL_SCANCODE_1: {
        //   // 1キー: C4（ド）
        //   float freq = MusicUtil::noteToFrequency(Note::C, 4);
        //   synthesizer_->getOscillator().setWaveType(ocillatorWaveType_);
        //   synthesizer_->noteOn(freq, 0.3f);
        //   break;
        // }
        
        case SDL_SCANCODE_O: {
          // オシレータ切り替え
          switch(ocillatorWaveType_){
            case WaveType::Sine:
              ocillatorWaveType_ = WaveType::Square;
              break;
            case WaveType::Square:
              ocillatorWaveType_ = WaveType::Sawtooth;
              break;
            case WaveType::Sawtooth:
              ocillatorWaveType_ = WaveType::Noise;
              break;
            default:
              ocillatorWaveType_ = WaveType::Sine;
          }
          break;
        }

        case SDL_SCANCODE_0: {
          // 0キー: シンセとBGM一括停止
          sequencer_->stop();
          synthesizer_->noteOff();
          bgm_manager_.stop();
          break;
        }

        // BGMマネージャーのテスト用キー
        case SDL_SCANCODE_1: {
          // F1キー: BGM1を再生（2トラック: メロディー + ベース）
          bgm_manager_.play("bgm1");
          break;
        }
        case SDL_SCANCODE_2: {
          // F2キー: BGM2を再生（3トラック: メロディー + 和音1 + 和音2）
          bgm_manager_.play("bgm2");
          break;
        }
        case SDL_SCANCODE_3: {
          // F3キー: BGM3を再生（2トラック: リズミカルな曲）
          bgm_manager_.play("bgm3");
          break;
        }
        case SDL_SCANCODE_5: {
          // F5キー: 現在のBGMを停止
          bgm_manager_.stop();
          break;
        }
        case SDL_SCANCODE_6: {
          // F6キー: 現在のBGMを一時停止
          bgm_manager_.pause();
          break;
        }
        case SDL_SCANCODE_7: {
          // F7キー: 現在のBGMを再開
          bgm_manager_.resume();
          break;
        }
        case SDL_SCANCODE_LEFTBRACKET: {
          // [キー: BGMマスターボリュームを下げる
          bgm_master_volume_ -= 0.1f;
          if (bgm_master_volume_ < 0.0f) bgm_master_volume_ = 0.0f;
          bgm_manager_.setMasterVolume(bgm_master_volume_);
          break;
        }
        case SDL_SCANCODE_RIGHTBRACKET: {
          // ]キー: BGMマスターボリュームを上げる
          bgm_master_volume_ += 0.1f;
          if (bgm_master_volume_ > 1.0f) bgm_master_volume_ = 1.0f;
          bgm_manager_.setMasterVolume(bgm_master_volume_);
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
    SDL_RenderDebugText(renderer_, 10, 30, "1-3: BGM1-3, 5: Stop, 6: Pause, 7: Resume, []: Vol");

    // サウンドシンセサイザーとシーケンサーを更新
    synthesizer_->update();
    sequencer_->update();

    // BGMマネージャーを更新
    bgm_manager_.update();

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
    // VelocityMoveコンポーネントとDirectionコンポーネントを取得
    auto* velocity = player_->getComponent<VelocityMove>();
    auto* direction = player_->getComponent<DirectionComponent>();
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
    bool is_moving = false;

    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
      vy -= speed;
      is_moving = true;
      if (direction) direction->setDirection(Direction::Up);
    }
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
      vy += speed;
      is_moving = true;
      if (direction) direction->setDirection(Direction::Down);
    }
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
      vx -= speed;
      is_moving = true;
      if (direction) direction->setDirection(Direction::Left);
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
      vx += speed;
      is_moving = true;
      if (direction) direction->setDirection(Direction::Right);
    }

    // 速度を設定
    velocity->setVelocity(vx, vy);
  }

  /**
   * @brief BGMマネージャーを初期化
   */
  void initializeBGMManager() {

    // BGM1:
    {
      auto bgm = std::make_unique<MultiTrackSequencer>(2);
      bgm->setMasterVolume(bgm_master_volume_);
      bgm->setLoop(true, -1);  // 無限ループ
      // bgm->setUpdateInterval(1); // 精度上げる時用のメモ(デフォルト15ms)
      // track1:
      bgm->getSynthesizer(0)->getEnvelope().setADSR(0.01f, 0.1f, 0.5f, 0.1f);
      bgm->setTrackSequence(0,
        "t180 o3 l8 @1 v8"_mml
        "cc>c<c c>c<c<b- rb->b-<b- b-<b->cd"_mml
        "aa>a<a a>a<a<a- ra->a-<a- a-<a->b-<b-"_mml
      );
      // track2:
      bgm->getSynthesizer(1)->getEnvelope().setADSR(0.01f, 0.1f, 0.5f, 0.1f);
      bgm->setTrackSequence(1,
        "t180 o4 l8 @2 v10"_mml
        "edcd efrg rgrg fgeg"_mml
        "fefg ab-r>c rcrc< b-rb-r"_mml
      );
      bgm_manager_.registerBGM("bgm1", std::move(bgm));
    }

    // BGM2:
    {
      auto bgm = std::make_unique<MultiTrackSequencer>(3);
      bgm->setMasterVolume(bgm_master_volume_);
      bgm->setLoop(true, -1);  // 無限ループ
      // bgm->setUpdateInterval(1); // 精度上げる時用のメモ(デフォルト15ms)
      // track1:
      bgm->getSynthesizer(0)->getEnvelope().setADSR(0.01f, 0.1f, 0.5f, 0.1f);
      bgm->setTrackSequence(0,
        "t120 o3 l8 @1 v8"_mml
        "e4. d8  c4. f8  e4. c8  e4. r8 "_mml
        // "aa>a<a a>a<a<a- ra->a-<a- a-<a->b-<b-"_mml
      );
      // track2:
      bgm->getSynthesizer(1)->getEnvelope().setADSR(0.01f, 0.1f, 0.5f, 0.1f);
      bgm->setTrackSequence(1,
        "t120 o4 l8 @2 v5"_mml
        "c2 c2 c2. g4 "_mml
        // "fefg ab-r>c rcrc< b-rb-r"_mml
      );
      // track3:
      bgm->getSynthesizer(2)->getEnvelope().setADSR(0.01f, 0.1f, 0.5f, 0.1f);
      bgm->setTrackSequence(2,
        "t120 o3 l8 @0 v10"_mml
        "cgec cgec cgec cgec "_mml
        // "fefg ab-r>c rcrc< b-rb-r"_mml
      );
      bgm_manager_.registerBGM("bgm2", std::move(bgm));
    }

    // BGM3:
    {
      auto bgm = std::make_unique<MultiTrackSequencer>(3);
      bgm->setMasterVolume(bgm_master_volume_);
      bgm->setLoop(true, -1);  // 無限ループ
      // bgm->setUpdateInterval(93); // BPM160の16分音符ms = 60/160/4*1000 = 93.75
      bgm->setUpdateIntervalNS(93750); // → 体感の違いはないがこの指定が一番正確なはず。細かすぎるとCPU食うのでこの設定が落とし所として良さそう？
      bgm->getSynthesizer(0)->getEnvelope().setADSR(0.01f, 0.1f, 0.5f, 0.1f);
      bgm->setTrackSequence(0,
        "t160 o3 l16 @1 v6"_mml
        "ababaeab > cdcdedc<b ababaeab > cdcdefef"_mml
        // "aa>a<a a>a<a<a- ra->a-<a- a-<a->b-<b-"_mml
      );
      // track2:
      bgm->getSynthesizer(1)->getEnvelope().setADSR(0.01f, 0.1f, 0.5f, 0.1f);
      bgm->setTrackSequence(1,
        "t160 o3 l16 @2 v7"_mml
        "erererer frfrfrfr erererer drdrdrdr "_mml
        // "fefg ab-r>c rcrc< b-rb-r"_mml
      );
      // track3:
      bgm->getSynthesizer(2)->getEnvelope().setADSR(0.01f, 0.1f, 0.5f, 0.1f);
      bgm->setTrackSequence(2,
        "t160 o3 l16 @2 v7"_mml
        // "cgec cgec cgec cgec "_mml
        "crcrcrcr drdrdrdr crcrcrcr < brbrbrbr >"_mml
      );
      bgm_manager_.registerBGM("bgm3", std::move(bgm));
    }

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

      // 向き（初期は下向き）
      player->addComponent(std::make_unique<DirectionComponent>(Direction::Down));

      // スプライト描画（初期タイル: x=0, y=1）
      player->addComponent(std::make_unique<SpriteRenderer>(texture_, 8, 0, 1));

      // 向きごとのスプライトアニメーション
      std::vector<std::pair<int, int>> down_frames = {{0, 1}, {1, 1}};   // 下向き
      std::vector<std::pair<int, int>> up_frames = {{2, 1}, {3, 1}};     // 上向き
      std::vector<std::pair<int, int>> right_frames = {{4, 1}, {5, 1}};  // 右向き
      // 左向きは空（右向きを左右反転）

      player->addComponent(std::make_unique<SpriteAnimator>(down_frames, 500));
      player->addComponent(std::make_unique<DirectionalSpriteAnimator>(
          down_frames, up_frames, right_frames));

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
        10, 10, 40,
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
        10, 10, 50,
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
