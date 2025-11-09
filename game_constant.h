#pragma once

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

}  // namespace MyGame
