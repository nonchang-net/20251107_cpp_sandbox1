#ifndef GAME_SNAKE_H__
#define GAME_SNAKE_H__

#include <SDL3/SDL.h>

#include <iostream>

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

class SnakeGame {
 private:
  SnakeContext snake_ctx;
  Uint64 last_step;
  SDL_Renderer* renderer;

  int shift(int x, int y) {
    return x + y * SNAKE_GAME_WIDTH * SNAKE_CELL_MAX_BITS;
  }

  SnakeCell snake_cell_at(char x, char y) {
    const int shifted = shift(x, y);
    unsigned short range;
    SDL_memcpy(&range, snake_ctx.cells + (shifted / 8), sizeof(range));
    return (SnakeCell)((range >> (shifted % 8)) & SNAKE_CELL_SET_BITS);
  }

  void set_rect_xy_(SDL_FRect* r, short x, short y) {
    r->x = (float)(x * SNAKE_BLOCK_SIZE_IN_PIXELS);
    r->y = (float)(y * SNAKE_BLOCK_SIZE_IN_PIXELS);
  }

  void put_cell_at_(char x, char y, SnakeCell ct) {
    const int shifted = shift(x, y);
    const int adjust = shifted % 8;
    unsigned char* const pos = snake_ctx.cells + (shifted / 8);
    unsigned short range;
    SDL_memcpy(&range, pos, sizeof(range));
    range &= ~(SNAKE_CELL_SET_BITS << adjust); /* clear bits */
    range |= (ct & SNAKE_CELL_SET_BITS) << adjust;
    SDL_memcpy(pos, &range, sizeof(range));
  }

  int are_cells_full_() {
    return snake_ctx.occupied_cells == SNAKE_GAME_WIDTH * SNAKE_GAME_HEIGHT;
  }

  void new_food_pos_() {
    while (true) {
      const char x = (char)SDL_rand(SNAKE_GAME_WIDTH);
      const char y = (char)SDL_rand(SNAKE_GAME_HEIGHT);
      if (snake_cell_at(x, y) == SNAKE_CELL_NOTHING) {
        put_cell_at_(x, y, SNAKE_CELL_FOOD);
        break;
      }
    }
  }

  void snake_initialize() {
    int i;
    SDL_zeroa(snake_ctx.cells);
    snake_ctx.head_xpos = snake_ctx.tail_xpos = SNAKE_GAME_WIDTH / 2;
    snake_ctx.head_ypos = snake_ctx.tail_ypos = SNAKE_GAME_HEIGHT / 2;
    snake_ctx.next_dir = SNAKE_DIR_RIGHT;
    snake_ctx.inhibit_tail_step = snake_ctx.occupied_cells = 4;
    --snake_ctx.occupied_cells;
    put_cell_at_(snake_ctx.tail_xpos, snake_ctx.tail_ypos, SNAKE_CELL_SRIGHT);
    for (i = 0; i < 4; i++) {
      new_food_pos_();
      ++snake_ctx.occupied_cells;
    }
  }

  void snake_redir(SnakeDirection dir) {
    SnakeCell ct = snake_cell_at(snake_ctx.head_xpos, snake_ctx.head_ypos);
    if ((dir == SNAKE_DIR_RIGHT && ct != SNAKE_CELL_SLEFT) ||
        (dir == SNAKE_DIR_UP && ct != SNAKE_CELL_SDOWN) ||
        (dir == SNAKE_DIR_LEFT && ct != SNAKE_CELL_SRIGHT) ||
        (dir == SNAKE_DIR_DOWN && ct != SNAKE_CELL_SUP)) {
      snake_ctx.next_dir = dir;
    }
  }

  void wrap_around_(char* val, char max) {
    if (*val < 0) {
      *val = max - 1;
    } else if (*val > max - 1) {
      *val = 0;
    }
  }

  void snake_step() {
    const SnakeCell dir_as_cell = (SnakeCell)(snake_ctx.next_dir + 1);
    SnakeCell ct;
    char prev_xpos;
    char prev_ypos;
    /* Move tail forward */
    if (--snake_ctx.inhibit_tail_step == 0) {
      ++snake_ctx.inhibit_tail_step;
      ct = snake_cell_at(snake_ctx.tail_xpos, snake_ctx.tail_ypos);
      put_cell_at_(snake_ctx.tail_xpos, snake_ctx.tail_ypos,
                   SNAKE_CELL_NOTHING);
      switch (ct) {
        case SNAKE_CELL_SRIGHT:
          snake_ctx.tail_xpos++;
          break;
        case SNAKE_CELL_SUP:
          snake_ctx.tail_ypos--;
          break;
        case SNAKE_CELL_SLEFT:
          snake_ctx.tail_xpos--;
          break;
        case SNAKE_CELL_SDOWN:
          snake_ctx.tail_ypos++;
          break;
        default:
          break;
      }
      wrap_around_(&snake_ctx.tail_xpos, SNAKE_GAME_WIDTH);
      wrap_around_(&snake_ctx.tail_ypos, SNAKE_GAME_HEIGHT);
    }
    /* Move head forward */
    prev_xpos = snake_ctx.head_xpos;
    prev_ypos = snake_ctx.head_ypos;
    switch (snake_ctx.next_dir) {
      case SNAKE_DIR_RIGHT:
        ++snake_ctx.head_xpos;
        break;
      case SNAKE_DIR_UP:
        --snake_ctx.head_ypos;
        break;
      case SNAKE_DIR_LEFT:
        --snake_ctx.head_xpos;
        break;
      case SNAKE_DIR_DOWN:
        ++snake_ctx.head_ypos;
        break;
      default:
        break;
    }
    wrap_around_(&snake_ctx.head_xpos, SNAKE_GAME_WIDTH);
    wrap_around_(&snake_ctx.head_ypos, SNAKE_GAME_HEIGHT);
    /* Collisions */
    ct = snake_cell_at(snake_ctx.head_xpos, snake_ctx.head_ypos);
    if (ct != SNAKE_CELL_NOTHING && ct != SNAKE_CELL_FOOD) {
      snake_initialize();
      return;
    }
    put_cell_at_(prev_xpos, prev_ypos, dir_as_cell);
    put_cell_at_(snake_ctx.head_xpos, snake_ctx.head_ypos, dir_as_cell);
    if (ct == SNAKE_CELL_FOOD) {
      if (are_cells_full_()) {
        snake_initialize();
        return;
      }
      new_food_pos_();
      ++snake_ctx.inhibit_tail_step;
      ++snake_ctx.occupied_cells;
    }
  }

 public:
  SnakeGame(SDL_Renderer* renderer) : renderer(renderer) {
    snake_initialize();
    last_step = SDL_GetTicks();
  }

  SDL_AppResult handleKeyEvent(SDL_Scancode key_code) {
    // std::cout << key_code << std::endl;
    switch (key_code) {
      /* Quit. */
      case SDL_SCANCODE_ESCAPE:
      case SDL_SCANCODE_Q:
        return SDL_APP_SUCCESS;
      /* Restart the game as if the program was launched. */
      case SDL_SCANCODE_R:
        snake_initialize();
        break;
      /* Decide new direction of the snake. */
      case SDL_SCANCODE_RIGHT:
        snake_redir(SNAKE_DIR_RIGHT);
        break;
      case SDL_SCANCODE_UP:
        snake_redir(SNAKE_DIR_UP);
        break;
      case SDL_SCANCODE_LEFT:
        snake_redir(SNAKE_DIR_LEFT);
        break;
      case SDL_SCANCODE_DOWN:
        snake_redir(SNAKE_DIR_DOWN);
        break;
      default:
        break;
    }
    return SDL_APP_CONTINUE;
  }

  SDL_AppResult handleHatEvent(Uint8 hat) {
    switch (hat) {
      case SDL_HAT_RIGHT:
        snake_redir(SNAKE_DIR_RIGHT);
        break;
      case SDL_HAT_UP:
        snake_redir(SNAKE_DIR_UP);
        break;
      case SDL_HAT_LEFT:
        snake_redir(SNAKE_DIR_LEFT);
        break;
      case SDL_HAT_DOWN:
        snake_redir(SNAKE_DIR_DOWN);
        break;
      default:
        break;
    }
    return SDL_APP_CONTINUE;
  }

  SDL_AppResult update() {
    const Uint64 now = SDL_GetTicks();
    SDL_FRect r;
    unsigned i;
    unsigned j;
    int ct;

    // run game logic if we're at or past the time to run it.
    // if we're _really_ behind the time to run it, run it
    // several times.
    while ((now - last_step) >= STEP_RATE_IN_MILLISECONDS) {
      snake_step();
      last_step += STEP_RATE_IN_MILLISECONDS;
    }

    r.w = r.h = SNAKE_BLOCK_SIZE_IN_PIXELS;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    for (i = 0; i < SNAKE_GAME_WIDTH; i++) {
      for (j = 0; j < SNAKE_GAME_HEIGHT; j++) {
        ct = snake_cell_at(i, j);
        if (ct == SNAKE_CELL_NOTHING) continue;
        set_rect_xy_(&r, i, j);
        if (ct == SNAKE_CELL_FOOD)
          SDL_SetRenderDrawColor(renderer, 80, 80, 255, SDL_ALPHA_OPAQUE);
        else /* body */
          SDL_SetRenderDrawColor(renderer, 0, 128, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &r);
      }
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE); /*head*/
    set_rect_xy_(&r, snake_ctx.head_xpos, snake_ctx.head_ypos);
    SDL_RenderFillRect(renderer, &r);
    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
  }
};

#pragma endrigon SnakeGame

}  // namespace MyGame::SnakeGame

#endif