#include "test_impl_2.h"

#include "../game_constant.h"

namespace MyGame {

SDL_AppResult TestImpl2::handleSdlEvent(SDL_Event* event) {
  //   std::cout << "TestImpl2::handleSdlEvent() called!" << std::endl;
  return SDL_APP_CONTINUE;
}
SDL_AppResult TestImpl2::update() {
  //   std::cout << "TestImpl2::update() called!" << std::endl;

  // ポイント群ランダム移動test
  for (int i = 0; i < SDL_arraysize(points); i++) {
    points[i].x = (SDL_randf() * 440.0f) + 100.0f;
    points[i].y = (SDL_randf() * 280.0f) + 100.0f;
  }

  painter.Clear(33, 33, 33);
  painter.SetColor(0, 0, 255)->Rect(100, 100, 440, 280);

  // note: RenderPointsは括りだし失敗中。まあいらんかなw
  // painter->SetColor(255, 255, 255)->Points(points);
  painter.SetColor(255, 255, 255);
  SDL_RenderPoints(renderer, points.data(), points.size());

  painter.SetColor(255, 255, 0)
      ->Line(0, 0, MyGame::CANVAS_WIDTH, MyGame::CANVAS_HEIGHT)
      ->Line(0, MyGame::CANVAS_HEIGHT, MyGame::CANVAS_WIDTH, 0);

  SDL_RenderPresent(renderer);

  return SDL_APP_CONTINUE;
}
}  // namespace MyGame