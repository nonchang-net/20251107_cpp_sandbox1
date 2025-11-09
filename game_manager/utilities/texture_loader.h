#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <tuple>

namespace MyGame::Utilities {

/**
 * @brief SDLリソース用のカスタムデリータ
 */
struct SDLSurfaceDeleter {
  void operator()(SDL_Surface* surface) const {
    if (surface) SDL_DestroySurface(surface);
  }
};

struct SDLStringDeleter {
  void operator()(char* str) const {
    if (str) SDL_free(str);
  }
};

/**
 * @brief テクスチャをファイルから読み込む
 *
 * 指定されたPNGファイルからSDL_Textureを作成します。
 * SDL_GetBasePath()を基準に相対パスで指定できます。
 * ドット絵に適したニアレストネイバーフィルタリング（補間なし）が設定されます。
 *
 * @param renderer SDLレンダラー
 * @param filename 読み込むファイル名（SDL_GetBasePath()からの相対パス）
 * @return {テクスチャ（失敗時nullptr）, 幅, 高さ} のタプル
 *
 * @note 戻り値のSDL_Texture*は呼び出し側が所有権を持ち、
 *       使用後にSDL_DestroyTexture()で解放する必要があります。
 * @note テクスチャのスケールモードはSDL_SCALEMODE_NEARESTに設定されます。
 *       これによりドット絵を拡大してもぼやけず、くっきり表示されます。
 *
 * 使用例:
 * @code
 * auto [texture, width, height] = Utilities::load_texture(renderer, "images/sprite.png");
 * if (texture) {
 *   // テクスチャを使用
 *   SDL_DestroyTexture(texture);  // 使用後に解放
 * }
 * @endcode
 */
inline std::tuple<SDL_Texture*, int, int> load_texture(
    SDL_Renderer* renderer, const char* filename) {
  if (!renderer) {
    SDL_Log("Invalid parameters: renderer is null");
    return {nullptr, 0, 0};
  }
  if (!filename) {
    SDL_Log("Invalid parameters: filename is null");
    return {nullptr, 0, 0};
  }

  // パス文字列を自動管理
  char* png_path_raw = nullptr;
  SDL_asprintf(&png_path_raw, "%s%s", SDL_GetBasePath(), filename);
  std::unique_ptr<char, SDLStringDeleter> png_path(png_path_raw);

  if (!png_path) {
    SDL_Log("Failed to allocate path string");
    return {nullptr, 0, 0};
  }

  // サーフェスを読み込み（自動管理）
  std::unique_ptr<SDL_Surface, SDLSurfaceDeleter> surface(SDL_LoadPNG(png_path.get()));

  if (!surface) {
    SDL_Log("Failed to load PNG '%s': %s", filename, SDL_GetError());
    return {nullptr, 0, 0};
  }

  // サイズを取得
  int width = surface->w;
  int height = surface->h;

  // テクスチャを作成
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface.get());
  if (!texture) {
    SDL_Log("Failed to create texture from '%s': %s", filename, SDL_GetError());
    return {nullptr, 0, 0};
  }

  // ドット絵用にニアレストネイバーフィルタリングを設定（補間なし）
  if (!SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST)) {
    SDL_Log("Warning: Failed to set texture scale mode for '%s': %s", filename,
            SDL_GetError());
    // エラーでもテクスチャは使用可能なので続行
  }

  // サーフェスは自動的に解放される
  // テクスチャの所有権は呼び出し側に移譲
  return {texture, width, height};
}

}  // namespace MyGame::Utilities
