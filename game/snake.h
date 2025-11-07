#pragma once

#include <SDL3/SDL.h>

#include <iostream>

#include "../game_manager/game_impl.h"

namespace MyGame::SnakeGame {

const int STEP_RATE_IN_MILLISECONDS = 125;
const int SNAKE_BLOCK_SIZE_IN_PIXELS = 24;
const unsigned int SNAKE_GAME_WIDTH = 24U;
const unsigned int SNAKE_GAME_HEIGHT = 18U;
const int WINDOW_WIDTH = SNAKE_BLOCK_SIZE_IN_PIXELS * SNAKE_GAME_WIDTH;
const int WINDOW_HEIGHT = SNAKE_BLOCK_SIZE_IN_PIXELS * SNAKE_GAME_HEIGHT;
const int SNAKE_MATRIX_SIZE = SNAKE_GAME_WIDTH * SNAKE_GAME_HEIGHT;
const unsigned int SNAKE_CELL_MAX_BITS =
    3U; /* floor(log2(SNAKE_CELL_FOOD)) + 1 */
const int SNAKE_CELL_SET_BITS = (~(~0u << SNAKE_CELL_MAX_BITS));

typedef enum {
  SNAKE_CELL_NOTHING = 0U,
  SNAKE_CELL_SRIGHT = 1U,
  SNAKE_CELL_SUP = 2U,
  SNAKE_CELL_SLEFT = 3U,
  SNAKE_CELL_SDOWN = 4U,
  SNAKE_CELL_FOOD = 5U
} SnakeCell;

typedef enum {
  SNAKE_DIR_RIGHT,
  SNAKE_DIR_UP,
  SNAKE_DIR_LEFT,
  SNAKE_DIR_DOWN
} SnakeDirection;

// undone: これは独立クラスとすべきかな？ structで済むならその方が良いのかな
typedef struct {
  unsigned char cells[(SNAKE_MATRIX_SIZE * SNAKE_CELL_MAX_BITS) / 8U];
  char head_xpos;
  char head_ypos;
  char tail_xpos;
  char tail_ypos;
  char next_dir;
  char inhibit_tail_step;
  unsigned occupied_cells;
} SnakeContext;

#pragma region SnakeGame

class SnakeGame : public GameImpl {
 private:
  SnakeContext snake_ctx;
  Uint64 last_step;
  SDL_Renderer* renderer;

  SnakeCell snake_cell_at(char x, char y);
  void set_rect_xy_(SDL_FRect* r, short x, short y);
  void put_cell_at_(char x, char y, SnakeCell ct);
  int are_cells_full_();
  void new_food_pos_();
  void snake_initialize();
  void snake_redir(SnakeDirection dir);
  void snake_step();

 public:
  SnakeGame(SDL_Renderer* renderer);
  SDL_AppResult handleKeyEvent(SDL_Scancode key_code);
  // SDL_AppResult handleHatEvent(Uint8 hat);
  // SDL_AppResult handleUserEvent(SDL_Event* event);
  SDL_AppResult handleSdlEvent(SDL_Event*);
  SDL_AppResult update();
};

#pragma endrigon SnakeGame

}  // namespace MyGame::SnakeGame
