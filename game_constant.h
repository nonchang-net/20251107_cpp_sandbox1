#pragma once

#include <SDL3/SDL.h>

namespace MyGame {

// note: SDL APIはconst char*を要求
constexpr const char* APP_TITLE = "My SDL3 Sandbox1";
constexpr const char* VERSION_CODE = "0.0.1";
constexpr const char* APP_IDENTIFIER = "net.nonchang.sdl3.sandbox1";
constexpr long GAME_MANAGER_VERSION = 202511061700;
constexpr int CANVAS_WIDTH = 640;
constexpr int CANVAS_HEIGHT = 480;

// フレームレート設定
constexpr int TARGET_FPS = 60;  // 目標フレームレート（30, 60など）
constexpr bool ENABLE_VSYNC = true;  // VSync有効化（true推奨）

// SDL UserEvent定義
// タイムスケール関連のイベント
constexpr Uint32 EVENT_TIMESCALE_CHANGED = SDL_EVENT_USER + 0;  // タイムスケール変更イベント
constexpr Uint32 EVENT_PAUSE = SDL_EVENT_USER + 1;  // ポーズイベント
constexpr Uint32 EVENT_UNPAUSE = SDL_EVENT_USER + 2;  // アンポーズイベント

// タイムスケール変更要求イベント（ゲーム実装→game.cc）
constexpr Uint32 EVENT_REQUEST_SET_TIMESCALE = SDL_EVENT_USER + 10;  // タイムスケール設定要求
constexpr Uint32 EVENT_REQUEST_TOGGLE_PAUSE = SDL_EVENT_USER + 11;  // ポーズトグル要求

}  // namespace MyGame
