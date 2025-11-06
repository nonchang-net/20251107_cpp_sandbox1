#ifndef GAME_MANAGER_H__
#define GAME_MANAGER_H__

#include <SDL3/SDL.h>

#include <iostream>
#include <string>

#include "draw_helper.h"

namespace MyGame {

class GameManager {
 private:
  const long GAME_MANAGER_VERSION = 202511061700;
  const int CANVAS_WIDTH = 640;
  const int CANVAS_HEIGHT = 480;
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  DrawHelper* painter = NULL;

  SDL_FPoint points[500];

 public:
  const SDL_AppResult init();
  const SDL_AppResult update();
};

#pragma region INIT

const SDL_AppResult GameManager::init() {
  std::cout << "hello: ver " << GAME_MANAGER_VERSION << std::endl;

  SDL_SetAppMetadata("My SDL3 Sandbox1", "0.0.1", "net.nonchang.sdl3.sandbox1");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("examples/renderer/primitives", CANVAS_WIDTH,
                                   CANVAS_HEIGHT, SDL_WINDOW_RESIZABLE, &window,
                                   &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderLogicalPresentation(renderer, CANVAS_WIDTH, CANVAS_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  painter = new DrawHelper(renderer);

  return SDL_APP_CONTINUE;
}

#pragma endregion INIT

#pragma region UPDATE

const SDL_AppResult GameManager::update() {
  SDL_FRect rect;

  // ポイント群ランダム移動test
  for (int i = 0; i < SDL_arraysize(points); i++) {
    points[i].x = (SDL_randf() * 440.0f) + 100.0f;
    points[i].y = (SDL_randf() * 280.0f) + 100.0f;
  }

  painter->Clear(33, 33, 33);
  painter->SetColor(0, 0, 255)->Rect(100, 100, 440, 280);

  // note: RenderPointsは括りだし失敗中。まあいらんかなw
  // painter->SetColor(255, 255, 255)->Points(points);
  painter->SetColor(255, 255, 255);
  SDL_RenderPoints(renderer, points, SDL_arraysize(points));

  painter->SetColor(255, 255, 0)
      ->Line(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT)
      ->Line(0, CANVAS_HEIGHT, CANVAS_WIDTH, 0);

  SDL_RenderPresent(renderer);

  return SDL_APP_CONTINUE;
}

#pragma endregion UPDATE

}  // namespace MyGame

#endif