#pragma once

#include <SDL3/SDL.h>
namespace MyGame {

// TODO: GameImplインターフェースをconceptで表現できるとのこと
// template<typename T>
// concept GameImplementation = requires(T t, SDL_Event* e) {
//   { t.handleSdlEvent(e) } -> std::same_as<SDL_AppResult>;
//   { t.update() } -> std::same_as<SDL_AppResult>;
// };
class GameImpl {
 public:
  virtual ~GameImpl() = default;
  virtual SDL_AppResult handleSdlEvent(SDL_Event* event) = 0;
  virtual SDL_AppResult update() = 0;
};

}  // namespace MyGame