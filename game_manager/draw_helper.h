#pragma once

#include <SDL3/SDL.h>

namespace MyGame {

class DrawHelper {
  SDL_Renderer* renderer = NULL;

 public:
  DrawHelper(SDL_Renderer* renderer) : renderer(renderer) {}
  DrawHelper* SetColor(Uint8, Uint8, Uint8, Uint8);
  DrawHelper* SetColor(Uint8, Uint8, Uint8);
  DrawHelper* Clear(Uint8, Uint8, Uint8);
  DrawHelper* Rect(float, float, float, float);
  // DrawHelper* Points(const SDL_FPoint *points);
  DrawHelper* Line(float, float, float, float);
};

}  // namespace MyGame
