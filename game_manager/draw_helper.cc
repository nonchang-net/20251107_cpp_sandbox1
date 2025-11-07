
#include "draw_helper.h"

namespace MyGame {

DrawHelper* DrawHelper::SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
  return this;
}
DrawHelper* DrawHelper::SetColor(Uint8 r, Uint8 g, Uint8 b) {
  return SetColor(r, g, b, SDL_ALPHA_OPAQUE);
}
DrawHelper* DrawHelper::Clear(Uint8 r, Uint8 g, Uint8 b) {
  SetColor(r, g, b, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
  return this;
}
DrawHelper* DrawHelper::Rect(float x, float y, float w, float h) {
  SDL_FRect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_RenderFillRect(renderer, &rect);
  return this;
}
// DrawHelper* DrawHelper::Points(const SDL_FPoint *points){
//   SDL_RenderPoints(renderer, points, SDL_arraysize(points));
//   return this;
// }
DrawHelper* DrawHelper::Line(float x, float y, float w, float h) {
  SDL_RenderLine(renderer, x, y, w, h);
  return this;
}
}  // namespace MyGame
