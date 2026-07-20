#ifndef _APP_SNAKE_GAME_H_
#define _APP_SNAKE_GAME_H_

#include <stdbool.h>
#include "app_game_common.h"

#define SNAKE_GRID_SIZE   4
#define SNAKE_COLS        (128 / SNAKE_GRID_SIZE)
#define SNAKE_ROWS        (64 / SNAKE_GRID_SIZE)
#define SNAKE_MAX_LEN     (SNAKE_COLS * SNAKE_ROWS)

void snake_game_init(void);
void snake_game_loop(void);
void snake_game_tick(void);
void snake_game_on_exit(void);
void snake_game_request_exit(void);
bool snake_game_should_exit(void);
void snake_game_set_click(void);
void snake_game_turn_left(void);
void snake_game_turn_right(void);

#endif // _APP_SNAKE_GAME_H_
