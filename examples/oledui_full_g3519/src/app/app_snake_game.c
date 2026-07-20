/**
 * app_snake_game.c
 * 贪吃蛇游戏实现
 */

#include "app_snake_game.h"
#include "app_game_common.h"
#include "game_port.h"
#include "OLED.h"
#include "OLED_UI_Driver.h"
#include "stdlib.h"

#define SNAKE_COMBO_TIMEOUT  100
#define SNAKE_DYING_FRAMES   15
#define SNAKE_MOVE_INTERVAL  6

typedef enum {
    DIR_UP = 0,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT
} snake_dir_t;

typedef struct {
    uint8_t x;
    uint8_t y;
} snake_point_t;

static snake_point_t snake_body[SNAKE_MAX_LEN];
static uint16_t snake_len;
static snake_dir_t snake_dir;
static snake_point_t food;
static uint16_t move_timer;

static game_scene_t snake_scene = SCENE_TITLE;
static game_score_t snake_score;
static game_combo_display_t snake_combo_display = {0, 0, false};
static uint16_t scene_frame = 0;
static uint16_t dying_timer = 0;
static uint16_t combo_timeout = 0;
static uint16_t animation_frame = 0;

static bool exit_requested = false;
static bool click_input = false;
static int8_t turn_input = 0;

static void snake_title_tick(void);
static void snake_running_tick(void);
static void snake_dying_tick(void);
static void snake_result_tick(void);
static void spawn_food(void);
static bool is_body(uint8_t x, uint8_t y);
static void draw_snake(void);
static void draw_food(void);

static void spawn_food(void) {
    uint8_t fx, fy;
    do {
        fx = rand() % SNAKE_COLS;
        fy = rand() % SNAKE_ROWS;
    } while (is_body(fx, fy));
    food.x = fx;
    food.y = fy;
}

static bool is_body(uint8_t x, uint8_t y) {
    for (uint16_t i = 0; i < snake_len; i++) {
        if (snake_body[i].x == x && snake_body[i].y == y) return true;
    }
    return false;
}

static void draw_snake(void) {
    for (uint16_t i = 0; i < snake_len; i++) {
        int px = snake_body[i].x * SNAKE_GRID_SIZE;
        int py = snake_body[i].y * SNAKE_GRID_SIZE;
        if (i == 0) {
            OLED_DrawRectangle(px, py, SNAKE_GRID_SIZE, SNAKE_GRID_SIZE, OLED_FILLED);
        } else {
            OLED_DrawRectangle(px + 1, py + 1, SNAKE_GRID_SIZE - 2, SNAKE_GRID_SIZE - 2, OLED_FILLED);
        }
    }
}

static void draw_food(void) {
    int px = food.x * SNAKE_GRID_SIZE;
    int py = food.y * SNAKE_GRID_SIZE;
    OLED_DrawRectangle(px, py, SNAKE_GRID_SIZE, SNAKE_GRID_SIZE, OLED_UNFILLED);
    OLED_DrawPoint(px + 1, py + 1);
    OLED_DrawPoint(px + 2, py + 2);
}

void snake_game_init(void) {
    game_common_score_reset(&snake_score);
    game_common_load_save(STORAGE_ID_SNAKE, &snake_score);
    snake_combo_display.multiplier = 0;
    snake_combo_display.display_timer = 0;
    snake_combo_display.score_flash = false;
    combo_timeout = 0;

    snake_scene = SCENE_TITLE;
    scene_frame = 0;
    dying_timer = 0;
    animation_frame = 0;
    exit_requested = false;
    click_input = false;
    turn_input = 0;
    move_timer = 0;

    snake_len = 3;
    snake_dir = DIR_RIGHT;
    snake_body[0].x = SNAKE_COLS / 2;
    snake_body[0].y = SNAKE_ROWS / 2;
    snake_body[1].x = snake_body[0].x - 1;
    snake_body[1].y = snake_body[0].y;
    snake_body[2].x = snake_body[0].x - 2;
    snake_body[2].y = snake_body[0].y;

    srand(animation_frame + 7);
    spawn_food();
}

void snake_game_set_click(void) {
    click_input = true;
}

void snake_game_turn_left(void) {
    turn_input = -1;
}

void snake_game_turn_right(void) {
    turn_input = 1;
}

void snake_game_request_exit(void) {
    exit_requested = true;
}

bool snake_game_should_exit(void) {
    return exit_requested;
}

static void snake_title_tick(void) {
    scene_frame++;
    animation_frame++;

    if (port_encoder_get() != 0 || click_input) {
        click_input = false;
        snake_scene = SCENE_RUNNING;
        scene_frame = 0;
        return;
    }

    port_clear_screen();
    draw_snake();
    draw_food();
    game_common_title_render("SNAKE", 20, snake_score.high_score, scene_frame);
    port_update_screen();
}

static void snake_running_tick(void) {
    animation_frame++;

    // 输入处理
    int16_t enc = port_encoder_get();
    if (enc < 0 || turn_input < 0) {
        snake_dir = (snake_dir + 3) % 4;  // 左转
        turn_input = 0;
    } else if (enc > 0 || turn_input > 0) {
        snake_dir = (snake_dir + 1) % 4;  // 右转
        turn_input = 0;
    }

    // 移动计时
    move_timer++;
    if (move_timer < SNAKE_MOVE_INTERVAL) {
        // 不移动，只渲染
        port_clear_screen();
        draw_snake();
        draw_food();
        game_common_combo_display_render(&snake_combo_display, snake_score.current_score);
        port_update_screen();
        return;
    }
    move_timer = 0;

    // 计算新头部位置
    snake_point_t new_head = snake_body[0];
    switch (snake_dir) {
        case DIR_UP:    new_head.y--; break;
        case DIR_DOWN:  new_head.y++; break;
        case DIR_LEFT:  new_head.x--; break;
        case DIR_RIGHT: new_head.x++; break;
    }

    // 边界碰撞（穿墙模式关闭，撞墙死亡）
    if (new_head.x >= SNAKE_COLS || new_head.y >= SNAKE_ROWS) {
        snake_scene = SCENE_DYING;
        dying_timer = 0;
        return;
    }

    // 自身碰撞
    if (is_body(new_head.x, new_head.y)) {
        snake_scene = SCENE_DYING;
        dying_timer = 0;
        return;
    }

    // 吃食物判定
    bool ate = (new_head.x == food.x && new_head.y == food.y);

    // 移动蛇身
    if (!ate) {
        for (uint16_t i = snake_len - 1; i > 0; i--) {
            snake_body[i] = snake_body[i - 1];
        }
    } else {
        for (uint16_t i = snake_len; i > 0; i--) {
            snake_body[i] = snake_body[i - 1];
        }
        snake_len++;
        game_common_combo_hit(&snake_score, &snake_combo_display);
        game_common_add_score(&snake_score, 10);
        combo_timeout = 0;
        spawn_food();
    }
    snake_body[0] = new_head;

    // combo超时
    combo_timeout++;
    if (combo_timeout >= SNAKE_COMBO_TIMEOUT && snake_score.combo_count > 0) {
        game_common_combo_reset(&snake_score);
    }
    game_common_combo_display_tick(&snake_combo_display);

    // 渲染
    port_clear_screen();
    draw_snake();
    draw_food();
    game_common_combo_display_render(&snake_combo_display, snake_score.current_score);
    port_update_screen();
}

static void snake_dying_tick(void) {
    dying_timer++;
    if (dying_timer >= SNAKE_DYING_FRAMES) {
        game_common_save_if_needed(STORAGE_ID_SNAKE, &snake_score);
        snake_scene = SCENE_RESULT;
        scene_frame = 0;
        return;
    }
    port_clear_screen();
    if (dying_timer % 3 != 0) {
        draw_snake();
    }
    port_draw_num(93, 2, snake_score.current_score, 5, FONT_SMALL);
    port_update_screen();
}

static void snake_result_tick(void) {
    scene_frame++;

    if (scene_frame > 40 && (port_encoder_get() != 0 || click_input)) {
        click_input = false;
        snake_game_init();
        return;
    }

    game_common_result_render(&snake_score, scene_frame);
}

void snake_game_tick(void) {
    switch (snake_scene) {
        case SCENE_TITLE:
            snake_title_tick();
            break;
        case SCENE_RUNNING:
            snake_running_tick();
            break;
        case SCENE_DYING:
            snake_dying_tick();
            break;
        case SCENE_RESULT:
            snake_result_tick();
            break;
    }
}

void snake_game_on_exit(void) {
    port_clear_screen();
    port_update_screen();
}

void snake_game_loop(void) {
    snake_game_init();

    while (1) {
        if (exit_requested) {
            port_clear_screen();
            port_update_screen();
            break;
        }

        snake_game_tick();
        port_delay_ms(30);
    }
}
