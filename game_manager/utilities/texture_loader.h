#include <SDL3/SDL.h>

#include <tuple>

namespace MyGame::Utilities {

std::tuple<SDL_Texture*, int, int> load_texture(SDL_Renderer* renderer, const char* filename) {
  char* png_path = NULL;
  SDL_asprintf(&png_path, "%s%s", SDL_GetBasePath(), filename);
  SDL_Surface* surface = NULL;
  surface = SDL_LoadPNG(png_path);
  if (!surface) {
    SDL_Log("Couldn't load bitmap: %s", SDL_GetError());
    return {nullptr, 0, 0};
  }
  SDL_free(png_path);

  int width = surface->w;
  int height = surface->h;

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) {
    SDL_Log("Couldn't create static texture: %s", SDL_GetError());
    return {nullptr, 0, 0};
  }

  SDL_DestroySurface(surface);

  return {texture, width, height};
}

}  // namespace MyGame::Utilities
