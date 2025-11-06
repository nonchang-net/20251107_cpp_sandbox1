#ifndef TEST_IMPL_1_H__
#define TEST_IMPL_1_H__

#include <SDL3/SDL.h>

#include "../draw_helper.h"
#include "../game_constant.h"

namespace MyGame::TestImpl1 {

class TestImpl1 {
 private:
  SDL_FPoint points[500];

 public:
  const void update(SDL_Renderer& renderer, DrawHelper& painter);
};

const void TestImpl1::update(SDL_Renderer& renderer, DrawHelper& painter) {
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
  SDL_RenderPoints(&renderer, points, SDL_arraysize(points));

  painter.SetColor(255, 255, 0)
      ->Line(0, 0, MyGame::CANVAS_WIDTH, MyGame::CANVAS_HEIGHT)
      ->Line(0, MyGame::CANVAS_HEIGHT, MyGame::CANVAS_WIDTH, 0);

  SDL_RenderPresent(&renderer);
}

}  // namespace MyGame::TestImpl1

#endif