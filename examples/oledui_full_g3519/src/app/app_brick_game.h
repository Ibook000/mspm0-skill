#ifndef _APP_BRICK_GAME_H_
#define _APP_BRICK_GAME_H_

#include <stdbool.h>
#include "app_game_common.h"

#define BRICK_COLS      10
#define BRICK_ROWS      3
#define BRICK_WIDTH     11
#define BRICK_HEIGHT    4
#define BRICK_GAP       1
#define BRICK_TOP_Y     4

#define PADDLE_WIDTH    20
#define PADDLE_HEIGHT   3
#define PADDLE_Y        60
#define PADDLE_SPEED    4

#define BALL_SIZE       3
#define BALL_SPEED_X    2
#define BALL_SPEED_Y   -2

#define SCREEN_W        128
#define SCREEN_H        64

void brick_game_init(void);
void brick_game_loop(void);
void brick_game_tick(void);
void brick_game_on_exit(void);
void brick_game_request_exit(void);
bool brick_game_should_exit(void);
void brick_game_set_click(void);

#endif // _APP_BRICK_GAME_H_
