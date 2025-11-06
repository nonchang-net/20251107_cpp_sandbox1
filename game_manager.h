#ifndef GAME_MANAGER_H__
#define GAME_MANAGER_H__

#include <SDL3/SDL.h>

#include <iostream>
#include <string>

namespace MyGame {

class GameManager {
 private:
  const long GAME_MANAGER_VERSION = 202511061700;
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  SDL_FPoint points[500];

 public:
  const SDL_AppResult init();
  const SDL_AppResult update();
};

const SDL_AppResult GameManager::init() {
  std::cout << "hello: ver " << GAME_MANAGER_VERSION << std::endl;

  SDL_SetAppMetadata("My SDL3 Sandbox1", "0.0.1", "net.nonchang.sdl3.sandbox1");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("examples/renderer/primitives", 640, 480,
                                   SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderLogicalPresentation(renderer, 640, 480,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  // ポイント配列初期化
  for (int i = 0; i < SDL_arraysize(points); i++) {
    points[i].x = (SDL_randf() * 440.0f) + 100.0f;
    points[i].y = (SDL_randf() * 280.0f) + 100.0f;
  }

  return SDL_APP_CONTINUE;
}

const SDL_AppResult GameManager::update() {
  SDL_FRect rect;

  /* as you can see from this, rendering draws over whatever was drawn before
   * it. */
  SDL_SetRenderDrawColor(renderer, 33, 33, 33,
                         SDL_ALPHA_OPAQUE); /* dark gray, full alpha */
  SDL_RenderClear(renderer);                /* start with a blank canvas. */

  /* draw a filled rectangle in the middle of the canvas. */
  SDL_SetRenderDrawColor(renderer, 0, 0, 255,
                         SDL_ALPHA_OPAQUE); /* blue, full alpha */
  rect.x = rect.y = 100;
  rect.w = 440;
  rect.h = 280;
  SDL_RenderFillRect(renderer, &rect);

  /* draw some points across the canvas. */
  SDL_SetRenderDrawColor(renderer, 255, 255, 255,
                         SDL_ALPHA_OPAQUE); /* red, full alpha */
  SDL_RenderPoints(renderer, points, SDL_arraysize(points));

  /* draw a unfilled rectangle in-set a little bit. */
  SDL_SetRenderDrawColor(renderer, 0, 255, 0,
                         SDL_ALPHA_OPAQUE); /* green, full alpha */
  rect.x += 30;
  rect.y += 30;
  rect.w -= 60;
  rect.h -= 60;
  SDL_RenderRect(renderer, &rect);

  /* draw two lines in an X across the whole canvas. */
  SDL_SetRenderDrawColor(renderer, 255, 255, 0,
                         SDL_ALPHA_OPAQUE); /* yellow, full alpha */
  SDL_RenderLine(renderer, 0, 0, 640, 480);
  SDL_RenderLine(renderer, 0, 480, 640, 0);

  SDL_RenderPresent(renderer); /* put it all on the screen! */

  return SDL_APP_CONTINUE;
}

}  // namespace MyGame

#endif