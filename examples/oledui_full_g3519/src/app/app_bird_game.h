/**
 * app_bird_game.h
 * 水管鸟游戏头文件
 */

#ifndef _APP_BIRD_GAME_H_
#define _APP_BIRD_GAME_H_

#include <stdbool.h>
#include "app_game_common.h"

typedef struct {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    float y_speed;
} bird_game_object_t;

typedef struct {
    bird_game_object_t top;
    bird_game_object_t bottom;
    bool passed;
} pipe_pair_t;

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define BIRD_WIDTH      10
#define BIRD_HEIGHT     8
#define PIPE_WIDTH      12
#define PIPE_GAP_HEIGHT 24
#define PIPE_SPEED      1
#define GRAVITY         0.2f
#define JUMP_FORCE      -1.5f
#define MAX_PIPES       3

void game_set_jump(void);
void game_init(void);
void game_start(void);
void game_loop(void);
void game_request_exit(void);
bool game_should_exit(void);
void game_tick(void);
void game_on_exit(void);

#endif // _APP_BIRD_GAME_H_
