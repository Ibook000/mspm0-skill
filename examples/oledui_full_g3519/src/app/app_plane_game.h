/**
 * app_plane_game.h
 * 飞机大战游戏头文件
 */

#ifndef _APP_PLANE_GAME_H_
#define _APP_PLANE_GAME_H_

#include <stdbool.h>
#include "app_game_common.h"

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

#define PLAYER_WIDTH    12
#define PLAYER_HEIGHT   10
#define PLAYER_SPEED    6
#define PLAYER_INIT_X   2
#define PLAYER_INIT_Y   60

#define ENEMY_WIDTH     10
#define ENEMY_HEIGHT    8
#define ENEMY_SPEED_MIN 1
#define ENEMY_SPEED_MAX 3
#define MAX_ENEMIES     3

#define BULLET_WIDTH    4
#define BULLET_HEIGHT   2
#define BULLET_SPEED    5
#define MAX_BULLETS     4
#define FIRE_INTERVAL   15

#define PLANE_GAME_READY     0
#define PLANE_GAME_RUNNING   1
#define PLANE_GAME_OVER      2

typedef struct {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    unsigned int speed;
    bool active;
} plane_game_object_t;

typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
    bool click;
} plane_game_input_t;

void plane_game_set_click(void);
void plane_game_set_up(void);
void plane_game_set_down(void);
void plane_game_clear_controls(void);
void plane_game_init(void);
void plane_game_loop(void);
void plane_game_handle_input(void);
void plane_game_update(void);
void plane_game_render(void);
bool plane_game_check_collision(plane_game_object_t *obj1, plane_game_object_t *obj2);
void plane_game_generate_enemy(void);
void plane_game_fire_bullet(void);
void plane_game_over(void);
void plane_game_request_exit(void);
bool plane_game_should_exit(void);
void plane_game_tick(void);
void plane_game_on_exit(void);

#endif // _APP_PLANE_GAME_H_
