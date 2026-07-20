#ifndef _APP_DINO_GAME_H_
#define _APP_DINO_GAME_H_

#include <stdbool.h>
#include "app_game_common.h"

typedef struct {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    unsigned int speed;
} dino_game_object_t;

typedef enum {
    DINO_RUNNING,
    DINO_JUMPING,
    DINO_DUCKING,
    DINO_DEAD
} dino_state_t;

void dino_game_set_click(void);
void dino_game_init(void);
void dino_game_loop(void);
void dino_game_handle_input(void);
void dino_game_update(void);
void dino_game_render(void);
bool dino_game_check_collision(void);
void dino_game_generate_obstacle(void);
void dino_game_over(void);
void dino_game_request_exit(void);
bool dino_game_should_exit(void);
void dino_game_tick(void);
void dino_game_on_exit(void);

#endif // _APP_DINO_GAME_H_
